#include <doctest/doctest.h>
#include "combat/CombatSystem.h"

#include <cmath>

using namespace adventure;

namespace
{
	MeleeState activeSwing()
	{
		MeleeState m;
		m.phase = MeleePhase::Active;
		m.hitThisSwing = false;
		return m;
	}
	WeaponDef wpn()
	{
		WeaponDef d;
		d.reach = 1.9f;
		d.arc = 1.4f;
		d.damage = 25.0f;
		d.knockback = 6.0f;
		return d;
	}
	Enemy enemyAt(float x, float z, float hp = 50.0f)
	{
		Enemy e;
		e.position = {x, 0.0f, z};
		e.health = hp;
		return e;
	}
} // namespace

TEST_CASE("melee hits an enemy in front, within reach and arc")
{
	std::vector<Enemy> es{enemyAt(0, -1.0f)}; // 1 unit in front (yaw 0 => -Z)
	MeleeState m = activeSwing();
	EnemyTuning t;
	resolveMeleeHits(m, wpn(), Vector3{0, 0, 0}, 0.0f, es, t);
	CHECK(es[0].health == doctest::Approx(25.0));
	CHECK(es[0].state == EnemyState::Stagger);
	CHECK(m.hitThisSwing);
	CHECK(es[0].velocity.z < 0.0f); // knocked away (-Z)
}

TEST_CASE("melee misses an enemy behind or out of reach")
{
	EnemyTuning t;
	WeaponDef w = wpn();
	{
		std::vector<Enemy> es{enemyAt(0, 1.0f)}; // behind
		MeleeState m = activeSwing();
		resolveMeleeHits(m, w, Vector3{0, 0, 0}, 0.0f, es, t);
		CHECK(es[0].health == doctest::Approx(50.0));
		CHECK_FALSE(m.hitThisSwing);
	}
	{
		std::vector<Enemy> es{enemyAt(0, -5.0f)}; // too far
		MeleeState m = activeSwing();
		resolveMeleeHits(m, w, Vector3{0, 0, 0}, 0.0f, es, t);
		CHECK(es[0].health == doctest::Approx(50.0));
	}
}

TEST_CASE("one swing hits each enemy at most once")
{
	std::vector<Enemy> es{enemyAt(0, -1.0f)};
	MeleeState m = activeSwing();
	EnemyTuning t;
	WeaponDef w = wpn();
	resolveMeleeHits(m, w, Vector3{0, 0, 0}, 0.0f, es, t);
	resolveMeleeHits(m, w, Vector3{0, 0, 0}, 0.0f, es, t); // same swing
	CHECK(es[0].health == doctest::Approx(25.0));
}

TEST_CASE("a lethal hit kills the enemy")
{
	std::vector<Enemy> es{enemyAt(0, -1.0f, 20.0f)};
	MeleeState m = activeSwing();
	EnemyTuning t;
	resolveMeleeHits(m, wpn(), Vector3{0, 0, 0}, 0.0f, es, t);
	CHECK(es[0].health <= 0.0f);
	CHECK(es[0].state == EnemyState::Dead);
}

TEST_CASE("no hit outside the Active phase")
{
	std::vector<Enemy> es{enemyAt(0, -1.0f)};
	MeleeState m; // Idle
	EnemyTuning t;
	resolveMeleeHits(m, wpn(), Vector3{0, 0, 0}, 0.0f, es, t);
	CHECK(es[0].health == doctest::Approx(50.0));
}

TEST_CASE("enemy approaches; stagger and death timers expire")
{
	EnemyTuning t;
	std::vector<Enemy> es{enemyAt(0, -10.0f)};
	for (int i = 0; i < 60; ++i)
		updateEnemies(es, Vector3{0, 0, 0}, t, 1.0f / 60.0f);
	CHECK(std::fabs(es[0].position.z) < 10.0f); // moved closer

	es[0].state = EnemyState::Stagger;
	es[0].stateTimer = 0.1f;
	for (int i = 0; i < 12; ++i)
		updateEnemies(es, Vector3{0, 0, 0}, t, 1.0f / 60.0f);
	CHECK(es[0].state == EnemyState::Approach);

	es[0].state = EnemyState::Dead;
	es[0].stateTimer = 0.05f;
	es[0].active = true;
	for (int i = 0; i < 12; ++i)
		updateEnemies(es, Vector3{0, 0, 0}, t, 1.0f / 60.0f);
	CHECK_FALSE(es[0].active);
}
