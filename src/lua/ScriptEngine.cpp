#include "lua/ScriptEngine.h"
#include "lua/ScriptEngineImpl.h"

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>

namespace adventure
{
	static constexpr int kHookStep = 1000;                    // watchdog granularity (VM instructions)
	static constexpr long long kInstrBudget = 20'000'000;     // per-chunk instruction ceiling
	static constexpr std::size_t kMemCap = 64u * 1024 * 1024; // VM heap ceiling (bytes)

	// Custom Lua allocator with a hard cap: returns NULL past the cap so Lua raises "not enough
	// memory" (caught by pcall) instead of a script exhausting the process.
	static void* luaAlloc(void* ud, void* ptr, std::size_t osize, std::size_t nsize)
	{
		ScriptEngineImpl* impl = static_cast<ScriptEngineImpl*>(ud);
		if (nsize == 0)
		{
			std::free(ptr);
			if (ptr)
				impl->memUsed -= osize;
			return nullptr;
		}
		std::size_t next = impl->memUsed - (ptr ? osize : 0) + nsize;
		if (impl->memCap && next > impl->memCap)
			return nullptr; // over cap -> allocation fails
		void* np = std::realloc(ptr, nsize);
		if (np)
			impl->memUsed = next;
		return np;
	}

	// Stash the impl pointer in the lua_State's extraspace so the C hook can reach it.
	static ScriptEngineImpl*& implSlot(lua_State* L)
	{
		return *static_cast<ScriptEngineImpl**>(lua_getextraspace(L));
	}

	static void instrHook(lua_State* L, lua_Debug*)
	{
		ScriptEngineImpl* impl = implSlot(L);
		if (!impl || !impl->instrArmed)
			return;
		impl->instrCount += kHookStep;
		if (impl->instrBudget > 0 && impl->instrCount > impl->instrBudget)
			luaL_error(L, "instruction budget exceeded (watchdog)");
	}

	// Routed print(): sandbox output goes here, never to files.
	static int l_hostPrint(lua_State* L)
	{
		int n = lua_gettop(L);
		std::string out;
		for (int i = 1; i <= n; ++i)
		{
			size_t len = 0;
			const char* s = luaL_tolstring(L, i, &len); // pushes a string
			if (i > 1)
				out.push_back('\t');
			out.append(s, len);
			lua_pop(L, 1);
		}
		std::printf("[lua] %s\n", out.c_str());
		std::fflush(stdout);
		return 0;
	}

	ScriptEngine* ScriptEngine::instance()
	{
		static ScriptEngine engine;
		return &engine;
	}

	ScriptEngine::ScriptEngine()
	    : m_impl(std::make_unique<ScriptEngineImpl>())
	{
	}

	ScriptEngine::~ScriptEngine()
	{
		shutdown();
	}

	bool ScriptEngine::isReady() const
	{
		return m_impl->L != nullptr;
	}

	std::size_t ScriptEngine::luaMemoryBytes() const
	{
		if (!m_impl->L)
			return 0;
		// lua_gc reports KB in COUNT and the sub-KB remainder in COUNTB.
		const std::size_t kb = (std::size_t)lua_gc(m_impl->L, LUA_GCCOUNT);
		const std::size_t b = (std::size_t)lua_gc(m_impl->L, LUA_GCCOUNTB);
		return kb * 1024 + b;
	}

	double ScriptEngine::evalNumber(const std::string& expr, double def)
	{
		if (!m_impl->L)
			init();
		lua_State* L = m_impl->L;
		const int base = lua_gettop(L);

		std::string chunk = "return (" + expr + ")";
		if (luaL_loadbufferx(L, chunk.data(), chunk.size(), "eval", "t") != LUA_OK)
		{
			lua_settop(L, base);
			return def;
		}
		lua_rawgeti(L, LUA_REGISTRYINDEX, m_impl->sandboxRef);
		if (lua_setupvalue(L, -2, 1) == nullptr)
			lua_pop(L, 1);

		m_impl->instrArmed = true;
		m_impl->instrCount = 0;
		m_impl->instrBudget = kInstrBudget;
		int st = lua_pcall(L, 0, 1, 0);
		m_impl->instrArmed = false;

		double out = def;
		if (st == LUA_OK && lua_isnumber(L, -1))
			out = lua_tonumber(L, -1);
		lua_settop(L, base);
		return out;
	}

	void ScriptEngine::init()
	{
		if (m_impl->L)
			return;

		m_impl->memUsed = 0;
		m_impl->memCap = kMemCap;
		lua_State* L = lua_newstate(luaAlloc, m_impl.get(), 0); // seed 0: deterministic string hashing
		m_impl->L = L;
		implSlot(L) = m_impl.get();

		luaL_openlibs(L); // opens all std libs into _G; the sandbox only exposes safe ones
		lua_sethook(L, instrHook, LUA_MASKCOUNT, kHookStep);

		bindHostApi(L); // registers _G.host via LuaBridge (Bindings.cpp)
		buildSandbox();

		std::printf("[adventure] Lua ready (%s)\n", LUA_RELEASE);
		std::fflush(stdout);
	}

	void ScriptEngine::shutdown()
	{
		if (m_impl->L)
		{
			lua_close(m_impl->L);
			m_impl->L = nullptr;
		}
	}

	void ScriptEngine::buildSandbox()
	{
		lua_State* L = m_impl->L;

		lua_newtable(L); // [env]
		const int env = lua_gettop(L);

		// Safe base functions copied by name from _G.
		static const char* kSafe[] = {
		    "assert", "error", "ipairs", "pairs", "next", "select", "tonumber", "tostring", "type", "pcall", "xpcall", "rawequal", "rawget", "rawset", "rawlen", "unpack", nullptr};
		for (int i = 0; kSafe[i]; ++i)
		{
			lua_getglobal(L, kSafe[i]); // nil for names absent in 5.5 -> harmless
			lua_setfield(L, env, kSafe[i]);
		}

		// Safe libraries exposed whole.
		static const char* kLibs[] = {"math", "string", "table", nullptr};
		for (int i = 0; kLibs[i]; ++i)
		{
			lua_getglobal(L, kLibs[i]);
			lua_setfield(L, env, kLibs[i]);
		}

		// os: expose only time-reading helpers (no execute/getenv/exit/remove).
		lua_newtable(L);        // [env][osz]
		lua_getglobal(L, "os"); // [env][osz][os]
		static const char* kOs[] = {"time", "clock", "date", "difftime", nullptr};
		for (int i = 0; kOs[i]; ++i)
		{
			lua_getfield(L, -1, kOs[i]); // os.<k>
			lua_setfield(L, -3, kOs[i]); // osz.<k>
		}
		lua_pop(L, 1);              // pop real os -> [env][osz]
		lua_setfield(L, env, "os"); // env.os = osz

		// Routed print.
		lua_pushcfunction(L, l_hostPrint);
		lua_setfield(L, env, "print");

		// Expose the host API table if it was registered.
		lua_getglobal(L, "host");
		if (!lua_isnil(L, -1))
			lua_setfield(L, env, "host");
		else
			lua_pop(L, 1);

		// _G points at the sandbox itself (so `x = ...` assigns into the sandbox).
		lua_pushvalue(L, env);
		lua_setfield(L, env, "_G");

		// Stash env in the registry and clear the stack.
		lua_pushvalue(L, env);
		m_impl->sandboxRef = luaL_ref(L, LUA_REGISTRYINDEX);
		lua_settop(L, env - 1);
	}

	bool ScriptEngine::runString(const std::string& chunkName, const std::string& source)
	{
		if (!m_impl->L)
			init();
		lua_State* L = m_impl->L;
		const int base = lua_gettop(L);

		// Load as TEXT only ("t"): reject precompiled bytecode.
		int st = luaL_loadbufferx(L, source.data(), source.size(), chunkName.c_str(), "t");
		if (st != LUA_OK)
		{
			std::printf("[lua] load error (%s): %s\n", chunkName.c_str(), lua_tostring(L, -1));
			std::fflush(stdout);
			lua_settop(L, base);
			return false;
		}

		// Point the chunk's _ENV upvalue at the sandbox.
		lua_rawgeti(L, LUA_REGISTRYINDEX, m_impl->sandboxRef); // push env
		if (lua_setupvalue(L, -2, 1) == nullptr)
			lua_pop(L, 1); // guard: chunk with no _ENV

		m_impl->instrArmed = true;
		m_impl->instrCount = 0;
		m_impl->instrBudget = kInstrBudget;
		int pst = lua_pcall(L, 0, 0, 0);
		m_impl->instrArmed = false;

		if (pst != LUA_OK)
		{
			std::printf("[lua] runtime error (%s): %s\n", chunkName.c_str(), lua_tostring(L, -1));
			std::fflush(stdout);
			lua_settop(L, base);
			return false;
		}

		lua_settop(L, base);
		return true;
	}

	bool ScriptEngine::runFile(const std::string& path)
	{
		std::ifstream f(path, std::ios::binary);
		if (!f)
		{
			std::printf("[lua] cannot open %s\n", path.c_str());
			std::fflush(stdout);
			return false;
		}
		std::stringstream ss;
		ss << f.rdbuf();
		return runString(path, ss.str());
	}

	void ScriptEngine::selfTest()
	{
		if (!m_impl->L)
			init();

		static const char* kProbe =
		    "assert(io == nil, 'io leaked')\n"
		    "assert(require == nil, 'require leaked')\n"
		    "assert(load == nil, 'load leaked')\n"
		    "assert(loadfile == nil, 'loadfile leaked')\n"
		    "assert(dofile == nil, 'dofile leaked')\n"
		    "assert(debug == nil, 'debug leaked')\n"
		    "assert(package == nil, 'package leaked')\n"
		    "assert(os == nil or os.execute == nil, 'os.execute leaked')\n"
		    "assert(math and string and table, 'safe libs missing')\n";

		bool sandbox = runString("selftest.sandbox", kProbe);
		bool killed = !runString("selftest.watchdog", "while true do end");

		if (sandbox && killed)
			std::printf("[adventure] ScriptEngine selfTest OK\n");
		else
			std::printf("[adventure] ScriptEngine selfTest FAILED (sandbox=%d watchdog=%d)\n",
			            (int)sandbox,
			            (int)killed);
		std::fflush(stdout);
	}
} // namespace adventure
