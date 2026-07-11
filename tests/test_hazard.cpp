#include <doctest/doctest.h>
#include "combat/CombatSystem.h"
#include "world/BrushGeometry.h"
#include "world/MapParser.h"

using namespace adventure;
using namespace adventure::world;

namespace
{
	Hazard box(Vector3 mn, Vector3 mx, float dps)
	{
		return Hazard{mn, mx, dps};
	}
} // namespace

TEST_CASE("hazardContains: inside, outside, on the boundary")
{
	Hazard h = box({-1, 0, -1}, {1, 2, 1}, 30.0f);
	CHECK(hazardContains(h, Vector3{0, 1, 0}));       // inside
	CHECK(hazardContains(h, Vector3{1, 2, 1}));       // corner (inclusive)
	CHECK_FALSE(hazardContains(h, Vector3{2, 1, 0})); // outside x
	CHECK_FALSE(hazardContains(h, Vector3{0, 3, 0})); // above
}

TEST_CASE("hazardDamageAt sums overlapping volumes, zero when clear")
{
	std::vector<Hazard> hz{
	    box({-1, 0, -1}, {1, 2, 1}, 30.0f),
	    box({0, 0, 0}, {2, 2, 2}, 20.0f), // overlaps the first around (0.5,1,0.5)
	};
	CHECK(hazardDamageAt(hz, Vector3{0.5f, 1.0f, 0.5f}) == doctest::Approx(50.0));   // both
	CHECK(hazardDamageAt(hz, Vector3{-0.5f, 1.0f, -0.5f}) == doctest::Approx(30.0)); // first only
	CHECK(hazardDamageAt(hz, Vector3{5.0f, 1.0f, 5.0f}) == doctest::Approx(0.0));    // none
}

TEST_CASE("applyHazards damages a standing enemy and eventually kills it")
{
	EnemyTuning t;
	std::vector<Hazard> hz{box({-2, -1, -2}, {2, 3, 2}, 25.0f)};
	std::vector<Enemy> es(1);
	es[0].position = {0, 0.9f, 0}; // feet at ~0, inside the volume
	es[0].health = 50.0f;

	applyHazards(es, hz, t, 1.0f); // 1s -> 25 dmg
	CHECK(es[0].health == doctest::Approx(25.0));
	applyHazards(es, hz, t, 1.0f); // another 25 -> dead
	CHECK(es[0].health <= 0.0f);
	CHECK(es[0].state == EnemyState::Dead);
}

TEST_CASE("applyHazards ignores an enemy standing clear of every volume")
{
	EnemyTuning t;
	std::vector<Hazard> hz{box({-2, -1, -2}, {2, 3, 2}, 25.0f)};
	std::vector<Enemy> es(1);
	es[0].position = {10.0f, 0.9f, 0}; // well outside
	es[0].health = 50.0f;
	applyHazards(es, hz, t, 1.0f);
	CHECK(es[0].health == doctest::Approx(50.0));
}

TEST_CASE("buildHazards extracts trigger_hurt and keeps it out of solid geometry")
{
	// One solid worldspawn cube + one trigger_hurt cube.
	const char* src =
	    "{\n"
	    "\"classname\" \"worldspawn\"\n"
	    "{\n"
	    "( 0 0 0 ) ( 0 64 0 ) ( 0 0 64 ) floor [ 0 1 0 0 ] [ 0 0 -1 0 ] 0 1 1\n"
	    "( 64 0 0 ) ( 64 0 64 ) ( 64 64 0 ) floor [ 0 1 0 0 ] [ 0 0 -1 0 ] 0 1 1\n"
	    "( 0 0 0 ) ( 0 0 64 ) ( 64 0 0 ) floor [ 1 0 0 0 ] [ 0 0 -1 0 ] 0 1 1\n"
	    "( 0 64 0 ) ( 64 64 0 ) ( 0 64 64 ) floor [ 1 0 0 0 ] [ 0 0 -1 0 ] 0 1 1\n"
	    "( 0 0 0 ) ( 64 0 0 ) ( 0 64 0 ) floor [ 1 0 0 0 ] [ 0 1 0 0 ] 0 1 1\n"
	    "( 0 0 64 ) ( 0 64 64 ) ( 64 0 64 ) floor [ 1 0 0 0 ] [ 0 1 0 0 ] 0 1 1\n"
	    "}\n"
	    "}\n"
	    "{\n"
	    "\"classname\" \"trigger_hurt\"\n"
	    "\"dmg\" \"66\"\n"
	    "{\n"
	    "( 0 0 0 ) ( 0 32 0 ) ( 0 0 32 ) trigger [ 0 1 0 0 ] [ 0 0 -1 0 ] 0 1 1\n"
	    "( 32 0 0 ) ( 32 0 32 ) ( 32 32 0 ) trigger [ 0 1 0 0 ] [ 0 0 -1 0 ] 0 1 1\n"
	    "( 0 0 0 ) ( 0 0 32 ) ( 32 0 0 ) trigger [ 1 0 0 0 ] [ 0 0 -1 0 ] 0 1 1\n"
	    "( 0 32 0 ) ( 32 32 0 ) ( 0 32 32 ) trigger [ 1 0 0 0 ] [ 0 0 -1 0 ] 0 1 1\n"
	    "( 0 0 0 ) ( 32 0 0 ) ( 0 32 0 ) trigger [ 1 0 0 0 ] [ 0 1 0 0 ] 0 1 1\n"
	    "( 0 0 32 ) ( 0 32 32 ) ( 32 0 32 ) trigger [ 1 0 0 0 ] [ 0 1 0 0 ] 0 1 1\n"
	    "}\n"
	    "}\n";
	MapParseResult r = parseMap(src);
	REQUIRE(r.ok);

	WorldGeometry geo = buildWorld(r.data);
	CHECK(geo.collision.size() == 1); // only the worldspawn cube is solid; the trigger is skipped

	std::vector<Hazard> hz = buildHazards(r.data);
	REQUIRE(hz.size() == 1);
	CHECK(hz[0].damagePerSec == doctest::Approx(66.0));
	// A point inside the trigger box (engine space) reports its damage.
	const Vector3 inside = mapToEngine(Vector3{16, 16, 16});
	CHECK(hazardDamageAt(hz, inside) == doctest::Approx(66.0));
}
