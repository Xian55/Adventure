#include "Bench.h"
#include "lua/ScriptEngine.h"

// Lua is on the per-frame AI/encounter path, so its call cost is performance-relevant.
// Budgets are ceilings with headroom (measured in Debug, then tripled) — they catch regressions,
// they are not targets. Run mingw-release for representative absolute numbers.

// Cost of compiling + sandboxing + running a trivial chunk (load path).
ADV_BENCH(lua_runstring_trivial, 20000, 900.0)
{
	sScript->init();
	sScript->runString("bench", "local x = 1 + 1");
}

// Cost of running an already-simple arithmetic body (execution path under the watchdog).
ADV_BENCH(lua_runstring_math, 20000, 1200.0)
{
	sScript->init();
	sScript->runString("bench", "local s = 0 for i = 1, 8 do s = s + i * 2 end");
}

// Sampling the Lua VM heap size (called once per frame for the metrics overlay).
ADV_BENCH(lua_memory_query, 200000, 200.0)
{
	sScript->init();
	advbench::keep(sScript->luaMemoryBytes());
}
