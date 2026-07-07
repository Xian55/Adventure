#include <doctest/doctest.h>
#include "map_helpers.h"
#include "world/BrushGeometry.h"
#include "world/MapParser.h"

#include <cmath>

using namespace adventure::world;

// Golden snapshot: freeze the exact geometry a known map produces, so a refactor that shifts a vertex,
// flips a normal, or breaks the Z-up->Y-up conversion is caught immediately (not silently shipped).
TEST_CASE("golden: two-box geometry signature is frozen")
{
	// A 128-unit cube at origin + a 64-unit box offset in +X, both texture "t".
	std::string m = worldspawnOf(boxBrush(-64, 64, -64, 64, -64, 64) + boxBrush(128, 192, -32, 32, -32, 32));
	MapParseResult r = parseMap(m);
	REQUIRE(r.ok);
	WorldGeometry w = buildWorld(r.data);

	// One merged mesh (shared texture): 2 boxes * 24 verts / 36 indices.
	REQUIRE(w.meshes.size() == 1);
	CHECK(w.meshes[0].vertexCount() == 48);
	CHECK(w.meshes[0].indices.size() == 72);
	CHECK(w.meshes[0].colors.size() == 48 * 4);

	// Two convex collision brushes, 6 planes each.
	REQUIRE(w.collision.size() == 2);
	CHECK(w.collision[0].planes.size() == 6);
	CHECK(w.collision[1].planes.size() == 6);

	// AABBs use a 1 cm tolerance: catches a real shift / normal-flip / coord-swap (whole units) while
	// tolerating the sub-mm FP noise from clipping a huge seed quad at these coordinates.
	const double eps = 0.01;

	// Brush 0 AABB (engine): symmetric [-2,2]^3.
	CHECK(w.collision[0].min.x == doctest::Approx(-2.0).epsilon(eps));
	CHECK(w.collision[0].max.x == doctest::Approx(2.0).epsilon(eps));

	// Brush 1 pins the Z-up(map) -> Y-up(engine) conversion: engine = (x, z, -y) / 32.
	// map x[128,192] -> engine x[4,6]; map z[-32,32] -> engine y[-1,1]; map y[-32,32] -> engine z[-1,1].
	CHECK(w.collision[1].min.x == doctest::Approx(4.0).epsilon(eps));
	CHECK(w.collision[1].max.x == doctest::Approx(6.0).epsilon(eps));
	CHECK(w.collision[1].min.y == doctest::Approx(-1.0).epsilon(eps));
	CHECK(w.collision[1].max.y == doctest::Approx(1.0).epsilon(eps));
	CHECK(w.collision[1].min.z == doctest::Approx(-1.0).epsilon(eps));
	CHECK(w.collision[1].max.z == doctest::Approx(1.0).epsilon(eps));

	// Plane-normal signature: 6 axis-aligned unit normals -> sum of |components| == 6.
	float s = 0.0f;
	for (Vector4 pl : w.collision[0].planes)
		s += std::fabs(pl.x) + std::fabs(pl.y) + std::fabs(pl.z);
	CHECK(s == doctest::Approx(6.0));
}
