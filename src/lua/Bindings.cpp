// The one and only translation unit that includes LuaBridge3. All C++<->Lua marshaling lives
// here so the heavy header (and its big object file) is isolated. Errors cross a lua_pcall,
// so exceptions are disabled — Lua reports via longjmp, not C++ throw.
#define LUABRIDGE_HAS_EXCEPTIONS 0

#include "lua/ScriptEngineImpl.h"   // pulls in the extern "C" Lua headers
#include "LuaBridge.h"

#include <cstdio>
#include <string>

namespace adventure
{
	// Minimal host surface for M0 — proves the LuaBridge boundary compiles and links.
	// Real gameplay bindings (entity handles, weapon/enemy defs) attach here later.
	static void host_log(const std::string& msg)
	{
		std::printf("[host] %s\n", msg.c_str());
		std::fflush(stdout);
	}

	void bindHostApi(lua_State* L)
	{
		luabridge::getGlobalNamespace(L)
			.beginNamespace("host")
				.addFunction("log", &host_log)
			.endNamespace();
	}
}
