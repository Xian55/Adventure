#pragma once
#include <cstddef>
#include <string>
#include <memory>

// Sandboxed Lua VM (minilua 5.5) behind a pimpl; sScript singleton. See src/lua/CLAUDE.md.
namespace adventure
{
	struct ScriptEngineImpl;

	class ScriptEngine
	{
	public:
		static ScriptEngine* instance();

		void init(); // create VM + sandbox (idempotent)
		void shutdown();
		bool isReady() const;

		// Run untrusted Lua TEXT under the sandbox env, with the instruction watchdog armed
		// and errors isolated via pcall. Returns false on load/runtime error (logged).
		bool runString(const std::string& chunkName, const std::string& source);
		bool runFile(const std::string& path);

		// Current Lua VM heap usage in bytes (for the metrics overlay).
		std::size_t luaMemoryBytes() const;

		// Evaluate `return (expr)` in the sandbox; return the number, or def on error/non-number.
		double evalNumber(const std::string& expr, double def);

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
} // namespace adventure

#define sScript adventure::ScriptEngine::instance()
