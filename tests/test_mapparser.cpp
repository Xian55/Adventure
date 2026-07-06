#include <doctest/doctest.h>
#include "world/MapParser.h"

using namespace adventure::world;

namespace
{
	const char* kCube =
	    "{\n"
	    "\"classname\" \"worldspawn\"\n"
	    "{\n"
	    "( -64 -64 -64 ) ( -64 -64 64 ) ( -64 64 -64 ) stone [ 0 1 0 0 ] [ 0 0 -1 0 ] 0 1 1\n"
	    "( 64 64 64 ) ( 64 -64 64 ) ( 64 64 -64 ) stone [ 0 1 0 0 ] [ 0 0 -1 0 ] 0 1 1\n"
	    "( -64 -64 -64 ) ( 64 -64 -64 ) ( -64 -64 64 ) stone [ 1 0 0 0 ] [ 0 0 -1 0 ] 0 1 1\n"
	    "( 64 64 64 ) ( -64 64 64 ) ( 64 64 -64 ) stone [ 1 0 0 0 ] [ 0 0 -1 0 ] 0 1 1\n"
	    "( -64 -64 -64 ) ( -64 64 -64 ) ( 64 -64 -64 ) stone [ 1 0 0 0 ] [ 0 1 0 0 ] 0 1 1\n"
	    "( 64 64 64 ) ( 64 -64 64 ) ( -64 64 64 ) stone [ 1 0 0 0 ] [ 0 1 0 0 ] 0 1 1\n"
	    "}\n"
	    "}\n"
	    "{\n"
	    "\"classname\" \"info_player_start\"\n"
	    "\"origin\" \"0 0 128\"\n"
	    "\"angle\" \"90\"\n"
	    "}\n";
}

TEST_CASE("parses entities, brushes, and faces")
{
	MapParseResult r = parseMap(kCube);
	REQUIRE(r.ok);
	REQUIRE(r.data.entities.size() == 2);

	const Entity& world = r.data.entities[0];
	CHECK(world.classname == "worldspawn");
	REQUIRE(world.brushes.size() == 1);
	CHECK(world.brushes[0].faces.size() == 6);
	CHECK(world.brushes[0].faces[0].texture == "stone");
}

TEST_CASE("reads point-entity key/values")
{
	MapParseResult r = parseMap(kCube);
	REQUIRE(r.ok);
	const Entity* spawn = r.data.first("info_player_start");
	REQUIRE(spawn != nullptr);
	CHECK(spawn->number("angle") == doctest::Approx(90.0));
	Vector3 o = spawn->vec3("origin");
	CHECK(o.x == doctest::Approx(0.0));
	CHECK(o.z == doctest::Approx(128.0));
}

TEST_CASE("face plane points and Valve texture axes are parsed")
{
	MapParseResult r = parseMap(kCube);
	REQUIRE(r.ok);
	const Face& f = r.data.entities[0].brushes[0].faces[0];
	CHECK(f.p[0].x == doctest::Approx(-64.0));
	CHECK(f.uAxis.y == doctest::Approx(1.0));
	CHECK(f.vAxis.z == doctest::Approx(-1.0));
	CHECK(f.scaleX == doctest::Approx(1.0));
}

TEST_CASE("empty input is valid and yields no entities")
{
	MapParseResult r = parseMap("");
	CHECK(r.ok);
	CHECK(r.data.entities.empty());
}

TEST_CASE("comments are ignored")
{
	MapParseResult r = parseMap("// a comment\n{\n\"classname\" \"worldspawn\"\n}\n");
	REQUIRE(r.ok);
	REQUIRE(r.data.entities.size() == 1);
	CHECK(r.data.entities[0].classname == "worldspawn");
}

TEST_CASE("malformed input fails cleanly, never throws")
{
	CHECK_FALSE(parseMap("{ \"classname\" \"worldspawn\"").ok);      // unterminated entity
	CHECK_FALSE(parseMap("{ \"key\" }").ok);                         // key without value
	CHECK_FALSE(parseMap("{ { ( 0 0 0 ) ( 1 0 0 ) ( 0 1 0 ) t").ok); // truncated brush/face
}
