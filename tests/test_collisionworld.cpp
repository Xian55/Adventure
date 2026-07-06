#include <doctest/doctest.h>
#include "map_helpers.h"
#include "world/BrushGeometry.h"
#include "world/CollisionWorld.h"
#include "world/MapParser.h"

using namespace adventure::world;

namespace
{
	// A 128-unit solid cube -> engine cube [-2,2]^3.
	CollisionWorld cubeWorld()
	{
		MapParseResult r = parseMap(boxMap(-64, 64, -64, 64, -64, 64));
		REQUIRE(r.ok);
		CollisionWorld w;
		w.build(buildWorld(r.data));
		return w;
	}

	const Vector3 kSmall = {0.1f, 0.1f, 0.1f};
} // namespace

TEST_CASE("build copies the collision brushes")
{
	CollisionWorld w = cubeWorld();
	CHECK(w.brushCount() == 1);
}

TEST_CASE("a box inside the solid overlaps")
{
	CollisionWorld w = cubeWorld();
	CHECK(w.overlaps(Vector3{0, 0, 0}, kSmall));
}

TEST_CASE("a box far outside does not overlap")
{
	CollisionWorld w = cubeWorld();
	CHECK_FALSE(w.overlaps(Vector3{10, 0, 0}, kSmall));
	CHECK_FALSE(w.overlaps(Vector3{0, 10, 0}, kSmall));
}

TEST_CASE("a box straddling a face overlaps; just past it does not")
{
	CollisionWorld w = cubeWorld();
	// Face at x=2. Center 2.05 +/- 0.1 spans 1.95..2.15 -> part inside the solid.
	CHECK(w.overlaps(Vector3{2.05f, 0, 0}, kSmall));
	// Center 2.2 +/- 0.1 spans 2.1..2.3 -> fully outside.
	CHECK_FALSE(w.overlaps(Vector3{2.2f, 0, 0}, kSmall));
}

TEST_CASE("larger half-extents expand the overlap region (Minkowski)")
{
	CollisionWorld w = cubeWorld();
	// Center at 2.4 misses with a small box but hits with a 0.5 half-extent.
	CHECK_FALSE(w.overlaps(Vector3{2.4f, 0, 0}, kSmall));
	CHECK(w.overlaps(Vector3{2.4f, 0, 0}, Vector3{0.5f, 0.5f, 0.5f}));
}

TEST_CASE("empty world never overlaps")
{
	CollisionWorld w;
	CHECK(w.brushCount() == 0);
	CHECK_FALSE(w.overlaps(Vector3{0, 0, 0}, kSmall));
}
