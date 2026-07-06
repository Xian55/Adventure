#include <doctest/doctest.h>
#include "lua/ScriptEngine.h"

// Adversarial tests for the Lua sandbox — a security boundary, so these actively try to escape or
// exhaust it and assert containment. If a future binding punches a hole, one of these should fail.
using namespace adventure;

TEST_CASE("sandbox: dynamic code loaders are absent")
{
	sScript->init();
	CHECK(sScript->runString("t", "assert(load == nil)"));
	CHECK(sScript->runString("t", "assert(loadstring == nil)"));
	CHECK(sScript->runString("t", "assert(loadfile == nil)"));
	CHECK(sScript->runString("t", "assert(dofile == nil)"));
}

TEST_CASE("sandbox: filesystem / process / module access is absent")
{
	sScript->init();
	CHECK(sScript->runString("t", "assert(io == nil)"));
	CHECK(sScript->runString(
	    "t", "assert(os.execute == nil and os.remove == nil and os.exit == nil and os.getenv == nil)"));
	CHECK(sScript->runString("t", "assert(require == nil and package == nil)"));
	CHECK(sScript->runString("t", "assert(debug == nil)"));
}

TEST_CASE("sandbox: _G is the sandbox env, not the real globals")
{
	sScript->init();
	CHECK(sScript->runString("t", "x = 5; assert(_G.x == 5)")); // globals land in the sandbox
	CHECK(sScript->runString("t", "assert(_G.io == nil)"));     // can't reach real io via _G
}

TEST_CASE("watchdog kills an infinite loop")
{
	sScript->init();
	CHECK_FALSE(sScript->runString("t", "while true do end"));
	CHECK(sScript->runString("t", "return 1 + 1")); // still usable
}

TEST_CASE("memory cap contains an allocation bomb")
{
	sScript->init();
	// Exponential string growth would OOM without a cap; the capped allocator makes Lua raise
	// "not enough memory", which pcall turns into a clean failure (no crash, no hang).
	CHECK_FALSE(sScript->runString("t", "local s = 'x' while true do s = s .. s end"));
	CHECK(sScript->runString("t", "return 1 + 1")); // engine survives
}

TEST_CASE("engine recovers from a script error")
{
	sScript->init();
	CHECK_FALSE(sScript->runString("t", "error('boom')"));
	CHECK(sScript->runString("t", "assert(1 == 1)"));
}
