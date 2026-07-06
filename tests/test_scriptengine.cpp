#include <doctest/doctest.h>
#include "lua/ScriptEngine.h"

using namespace adventure;

// The engine is a process-wide singleton; init() is idempotent so every case can call it.
TEST_CASE("sandbox hides dangerous libraries")
{
	sScript->init();
	CHECK(sScript->runString("t", "assert(io == nil)"));
	CHECK(sScript->runString("t", "assert(require == nil)"));
	CHECK(sScript->runString("t", "assert(load == nil)"));
	CHECK(sScript->runString("t", "assert(dofile == nil)"));
	CHECK(sScript->runString("t", "assert(debug == nil)"));
	CHECK(sScript->runString("t", "assert(package == nil)"));
	CHECK(sScript->runString("t", "assert(os == nil or os.execute == nil)"));
}

TEST_CASE("safe libraries are available")
{
	sScript->init();
	CHECK(sScript->runString("t", "assert(math and string and table)"));
	CHECK(sScript->runString("t", "assert(string.format('%d', 3) == '3')"));
}

TEST_CASE("watchdog kills an infinite loop")
{
	sScript->init();
	CHECK_FALSE(sScript->runString("t", "while true do end"));
}

TEST_CASE("valid chunks run; malformed chunks fail cleanly")
{
	sScript->init();
	CHECK(sScript->runString("t", "local x = 1 + 1 assert(x == 2)"));
	CHECK_FALSE(sScript->runString("t", "this is not lua"));
	CHECK_FALSE(sScript->runString("t", "error('boom')"));
}

TEST_CASE("globals persist across chunks in the shared sandbox env")
{
	sScript->init();
	CHECK(sScript->runString("t", "shared_probe = 42"));
	CHECK(sScript->runString("t", "assert(shared_probe == 42)"));
}

TEST_CASE("lua memory is reported")
{
	sScript->init();
	CHECK(sScript->luaMemoryBytes() > 0);
}
