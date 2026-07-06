#pragma once

// Shared internal state for the Lua engine. Included by ScriptEngine.cpp (core, raw C API) and
// Bindings.cpp (the LuaBridge boundary). NOT a public header — never included by gameplay code.
extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

namespace adventure
{
	struct ScriptEngineImpl
	{
		lua_State* L = nullptr;

		// Instruction-count watchdog: only bites while armed (around untrusted calls).
		bool instrArmed = false;
		long long instrCount = 0;
		long long instrBudget = 0;

		int sandboxRef = LUA_NOREF; // registry ref to the whitelisted _ENV table
	};

	// Implemented in Bindings.cpp (the only TU that includes LuaBridge). Registers the host API
	// (e.g. _G.host) which buildSandbox() then exposes inside the sandbox env.
	void bindHostApi(lua_State* L);
} // namespace adventure
