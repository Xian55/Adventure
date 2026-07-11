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

TEST_CASE("evalNumber evaluates expressions and falls back on error")
{
	sScript->init();
	CHECK(sScript->evalNumber("1 + 2", -1.0) == doctest::Approx(3.0));
	CHECK(sScript->evalNumber("math.floor(3.7)", -1.0) == doctest::Approx(3.0));
	CHECK(sScript->evalNumber("nosuchglobal.x", 9.0) == doctest::Approx(9.0)); // index nil -> def
	CHECK(sScript->evalNumber("'a string'", 5.0) == doctest::Approx(5.0));     // non-number -> def

	REQUIRE(sScript->runString("t", "tuning = { moveSpeed = 8.5 }"));
	CHECK(sScript->evalNumber("tuning.moveSpeed", 0.0) == doctest::Approx(8.5));
}

TEST_CASE("evalString reads strings and falls back on error/non-string")
{
	sScript->init();
	CHECK(sScript->evalString("'hello'", "def") == "hello");
	CHECK(sScript->evalString("'a' .. 'b'", "def") == "ab");
	CHECK(sScript->evalString("123", "def") == "def");            // number -> def
	CHECK(sScript->evalString("nosuchglobal.x", "def") == "def"); // index nil -> def

	REQUIRE(sScript->runString("t", "items = { sword = { name = 'Longsword' } }"));
	CHECK(sScript->evalString("items.sword.name", "?") == "Longsword");
}
