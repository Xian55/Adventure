#include <doctest/doctest.h>
#include "world/BrushGeometry.h"
#include "world/MapParser.h"

#include "raymath.h"

#include <cmath>

using namespace adventure::world;

namespace
{
	// A single 128-unit cube, one texture (crisp counts). Winding-agnostic.
	const char* kCube =
	    "{\n"
	    "\"classname\" \"worldspawn\"\n"
	    "{\n"
	    "( -64 -64 -64 ) ( -64 -64 64 ) ( -64 64 -64 ) t [ 0 1 0 0 ] [ 0 0 -1 0 ] 0 1 1\n"
	    "( 64 64 64 ) ( 64 -64 64 ) ( 64 64 -64 ) t [ 0 1 0 0 ] [ 0 0 -1 0 ] 0 1 1\n"
	    "( -64 -64 -64 ) ( 64 -64 -64 ) ( -64 -64 64 ) t [ 1 0 0 0 ] [ 0 0 -1 0 ] 0 1 1\n"
	    "( 64 64 64 ) ( -64 64 64 ) ( 64 64 -64 ) t [ 1 0 0 0 ] [ 0 0 -1 0 ] 0 1 1\n"
	    "( -64 -64 -64 ) ( -64 64 -64 ) ( 64 -64 -64 ) t [ 1 0 0 0 ] [ 0 1 0 0 ] 0 1 1\n"
	    "( 64 64 64 ) ( 64 -64 64 ) ( -64 64 64 ) t [ 1 0 0 0 ] [ 0 1 0 0 ] 0 1 1\n"
	    "}\n"
	    "}\n";

	WorldGeometry buildCube()
	{
		MapParseResult r = parseMap(kCube);
		REQUIRE(r.ok);
		return buildWorld(r.data);
	}
} // namespace

TEST_CASE("cube brush -> one texture mesh with 24 verts / 36 indices")
{
	WorldGeometry w = buildCube();
	REQUIRE(w.meshes.size() == 1);
	CHECK(w.meshes[0].texture == "t");
	CHECK(w.meshes[0].vertexCount() == 24); // 6 quad faces, per-face verts
	CHECK(w.meshes[0].indices.size() == 36);
	CHECK(w.meshes[0].colors.size() == 24 * 4);
}

TEST_CASE("cube -> one collision brush with 6 planes and the expected AABB")
{
	WorldGeometry w = buildCube();
	REQUIRE(w.collision.size() == 1);
	const CollisionBrush& cb = w.collision[0];
	CHECK(cb.planes.size() == 6);

	// 64 map units * (1/32) = 2.0 engine units, symmetric.
	CHECK(cb.min.x == doctest::Approx(-2.0));
	CHECK(cb.min.y == doctest::Approx(-2.0));
	CHECK(cb.min.z == doctest::Approx(-2.0));
	CHECK(cb.max.x == doctest::Approx(2.0));
	CHECK(cb.max.y == doctest::Approx(2.0));
	CHECK(cb.max.z == doctest::Approx(2.0));
}

TEST_CASE("all six collision planes are axis-aligned and face outward")
{
	WorldGeometry w = buildCube();
	const CollisionBrush& cb = w.collision[0];

	// The interior (origin) must be strictly inside every plane (dist < 0).
	for (Vector4 pl : cb.planes)
		CHECK(pl.w < 0.0f); // dist(pl, origin) == pl.w

	// Exactly the six axis directions, each a unit normal.
	int px = 0, nx = 0, py = 0, ny = 0, pz = 0, nz = 0;
	for (Vector4 pl : cb.planes)
	{
		Vector3 n = {pl.x, pl.y, pl.z};
		CHECK(Vector3Length(n) == doctest::Approx(1.0));
		if (n.x > 0.9f)
			++px;
		if (n.x < -0.9f)
			++nx;
		if (n.y > 0.9f)
			++py;
		if (n.y < -0.9f)
			++ny;
		if (n.z > 0.9f)
			++pz;
		if (n.z < -0.9f)
			++nz;
	}
	CHECK(px == 1);
	CHECK(nx == 1);
	CHECK(py == 1);
	CHECK(ny == 1);
	CHECK(pz == 1);
	CHECK(nz == 1);
}

TEST_CASE("every mesh vertex lies within the brush AABB")
{
	WorldGeometry w = buildCube();
	const MeshData& m = w.meshes[0];
	for (int i = 0; i < m.vertexCount(); ++i)
	{
		float x = m.positions[i * 3 + 0];
		float y = m.positions[i * 3 + 1];
		float z = m.positions[i * 3 + 2];
		CHECK(std::fabs(x) <= doctest::Approx(2.0));
		CHECK(std::fabs(y) <= doctest::Approx(2.0));
		CHECK(std::fabs(z) <= doctest::Approx(2.0));
	}
}

TEST_CASE("empty map yields empty geometry")
{
	WorldGeometry w = buildWorld(MapData{});
	CHECK(w.meshes.empty());
	CHECK(w.collision.empty());
}
