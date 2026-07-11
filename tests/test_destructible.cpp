#include <doctest/doctest.h>
#include "combat/Destructible.h"

#include <cmath>

using namespace adventure;

namespace
{
	Destructible propAt(float x, float z, float hp, LootKind loot = LootKind::None)
	{
		Destructible d;
		d.position = {x, 0.5f, z};
		d.health = d.maxHealth = hp;
		d.loot = loot;
		return d;
	}
} // namespace

TEST_CASE("a hit in range damages a prop and breaks it at zero health")
{
	PropTuning t;
	std::vector<Destructible> props{propAt(0, 0, 30.0f)};
	std::vector<Pickup> loot;

	int broke = damageProps(props, loot, Vector3{0, 0.5f, 0}, 0.6f, 20.0f, t);
	CHECK(broke == 0);
	CHECK(props[0].health == doctest::Approx(10.0));
	CHECK_FALSE(props[0].broken);

	broke = damageProps(props, loot, Vector3{0, 0.5f, 0}, 0.6f, 20.0f, t);
	CHECK(broke == 1);
	CHECK(props[0].broken);
	CHECK(loot.empty()); // no loot on this one
}

TEST_CASE("a container spills a health pickup when it breaks")
{
	PropTuning t;
	std::vector<Destructible> props{propAt(2, 0, 10.0f, LootKind::Health)};
	std::vector<Pickup> loot;
	damageProps(props, loot, Vector3{2, 0.5f, 0}, 0.6f, 50.0f, t);
	REQUIRE(loot.size() == 1);
	CHECK(loot[0].kind == LootKind::Health);
	CHECK(loot[0].position.x == doctest::Approx(2.0));
}

TEST_CASE("out-of-range and already-broken props are ignored")
{
	PropTuning t;
	std::vector<Destructible> props{propAt(0, 0, 10.0f), propAt(10, 0, 10.0f)};
	std::vector<Pickup> loot;
	damageProps(props, loot, Vector3{0, 0.5f, 0}, 0.6f, 50.0f, t); // breaks #0 only
	CHECK(props[0].broken);
	CHECK_FALSE(props[1].broken);

	props[0].health = 5.0f; // pretend it had health; a second blast must not re-break it
	int broke = damageProps(props, loot, Vector3{0, 0.5f, 0}, 0.6f, 50.0f, t);
	CHECK(broke == 0);
}

TEST_CASE("debris despawns the prop after the timer")
{
	PropTuning t;
	t.debrisTime = 0.1f;
	std::vector<Destructible> props{propAt(0, 0, 10.0f)};
	std::vector<Pickup> loot;
	damageProps(props, loot, Vector3{0, 0.5f, 0}, 0.6f, 50.0f, t);
	CHECK(props[0].active);
	for (int i = 0; i < 8; ++i)
		updateProps(props, t, 1.0f / 60.0f);
	CHECK_FALSE(props[0].active);
}

TEST_CASE("walking over a health pickup heals, clamped to max")
{
	PropTuning t;
	t.healAmount = 25.0f;
	std::vector<Pickup> pickups{Pickup{{0, 0.5f, 0}, LootKind::Health, true}};
	float hp = 60.0f;
	collectPickups(pickups, Vector3{0, 0.5f, 0.3f}, hp, 100.0f, t);
	CHECK(hp == doctest::Approx(85.0));
	CHECK_FALSE(pickups[0].active);

	pickups = {Pickup{{0, 0.5f, 0}, LootKind::Health, true}};
	hp = 90.0f;
	collectPickups(pickups, Vector3{0, 0.5f, 0}, hp, 100.0f, t);
	CHECK(hp == doctest::Approx(100.0)); // clamped, not 115
}

TEST_CASE("a distant pickup is not collected")
{
	PropTuning t;
	std::vector<Pickup> pickups{Pickup{{0, 0.5f, 0}, LootKind::Health, true}};
	float hp = 50.0f;
	collectPickups(pickups, Vector3{5, 0.5f, 0}, hp, 100.0f, t);
	CHECK(hp == doctest::Approx(50.0));
	CHECK(pickups[0].active);
}

TEST_CASE("an intact prop blocks an actor, pushing it out to the surface")
{
	std::vector<Destructible> props(1);
	props[0].position = {0, 0.5f, 0};
	props[0].radius = 0.4f;
	props[0].height = 1.0f;

	Vector3 pos = {0.5f, 0.9f, 0.0f}; // overlapping (0.5 < radius 0.3 + 0.4 = 0.7)
	resolveActorProps(pos, 0.3f, 1.8f, props);
	const float d = std::sqrt(pos.x * pos.x + pos.z * pos.z);
	CHECK(d == doctest::Approx(0.7).epsilon(0.01)); // pushed to exactly radius sum
	CHECK(pos.z == doctest::Approx(0.0));           // shoved straight out along the approach axis
}

TEST_CASE("a clear, broken, or vertically-disjoint prop does not block")
{
	std::vector<Destructible> props(1);
	props[0].position = {0, 0.5f, 0};
	props[0].radius = 0.4f;
	props[0].height = 1.0f;

	{
		Vector3 pos = {2.0f, 0.9f, 0.0f}; // far away
		resolveActorProps(pos, 0.3f, 1.8f, props);
		CHECK(pos.x == doctest::Approx(2.0));
	}
	{
		props[0].broken = true;
		Vector3 pos = {0.5f, 0.9f, 0.0f}; // overlapping but the prop is rubble
		resolveActorProps(pos, 0.3f, 1.8f, props);
		CHECK(pos.x == doctest::Approx(0.5));
		props[0].broken = false;
	}
	{
		Vector3 pos = {0.5f, 5.0f, 0.0f}; // standing on a ledge above the prop
		resolveActorProps(pos, 0.3f, 1.8f, props);
		CHECK(pos.x == doctest::Approx(0.5));
	}
}
