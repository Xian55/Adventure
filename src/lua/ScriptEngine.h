#pragma once
#include <cstddef>
#include <string>
#include <memory>

// Sandboxed Lua engine. The VM (minilua / Lua 5.5) and LuaBridge are hidden behind a pimpl so
// the Lua C headers never leak into gameplay code — only src/lua/*.cpp pay that include cost.
// Singleton via the `sScript` macro, mirroring Emberfire's sLua / sConfig pattern.
namespace adventure
{
	struct ScriptEngineImpl;

	class ScriptEngine
	{
		public:
			static ScriptEngine* instance();

			void init();       // create VM + sandbox (idempotent)
			void shutdown();
			bool isReady() const;

			// Run untrusted Lua TEXT under the sandbox env, with the instruction watchdog armed
			// and errors isolated via pcall. Returns false on load/runtime error (logged).
			bool runString(const std::string& chunkName, const std::string& source);
			bool runFile(const std::string& path);

			// Current Lua VM heap usage in bytes (for the metrics overlay).
			std::size_t luaMemoryBytes() const;

			// Boot proof: asserts dangerous libs (io/os.execute/require/load/dofile/debug/package)
			// are absent from the sandbox and that the watchdog kills an infinite loop.
			void selfTest();

		private:
			ScriptEngine();
			~ScriptEngine();
			ScriptEngine(const ScriptEngine&) = delete;
			ScriptEngine& operator=(const ScriptEngine&) = delete;

			void buildSandbox();

			std::unique_ptr<ScriptEngineImpl> m_impl;
	};
}

#define sScript adventure::ScriptEngine::instance()
