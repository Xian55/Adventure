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
	PlayerTarget targetAt(Vector3 pos, float* hp = nullptr, bool shield = false, float yaw = 0.0f)
	{
		PlayerTarget p;
		p.pos = pos;
		p.yaw = yaw;
		p.shieldRaised = shield;
		p.health = hp;
		return p;
	}
	void step(std::vector<Enemy>& es, PlayerTarget& p, const EnemyTuning& t, int frames)
	{
		for (int i = 0; i < frames; ++i)
			updateEnemies(es, p, t, 1.0f / 60.0f);
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

TEST_CASE("the same swing that hits enemies also smashes a prop in its arc")
{
	EnemyTuning t;
	PropTuning pt;
	std::vector<Enemy> es; // no enemies present
	std::vector<Destructible> props(1);
	props[0].position = {0, 0.5f, -1.0f}; // 1 unit in front (yaw 0 => -Z)
	props[0].health = props[0].maxHealth = 20.0f;
	std::vector<Pickup> loot;
	MeleeState m = activeSwing();
	resolveMeleeHits(m, wpn(), Vector3{0, 0, 0}, 0.0f, es, t, 1.0f, &props, &loot, &pt);
	CHECK(props[0].health == doctest::Approx(20.0 - 25.0)); // wpn damage 25 applied
	CHECK(props[0].broken);
	CHECK(m.hitThisSwing);

	// A prop behind the player is untouched.
	std::vector<Destructible> behind(1);
	behind[0].position = {0, 0.5f, 1.0f};
	behind[0].health = behind[0].maxHealth = 20.0f;
	MeleeState m2 = activeSwing();
	resolveMeleeHits(m2, wpn(), Vector3{0, 0, 0}, 0.0f, es, t, 1.0f, &behind, &loot, &pt);
	CHECK(behind[0].health == doctest::Approx(20.0));
	CHECK_FALSE(behind[0].broken);
}

TEST_CASE("kick shoves front enemies back and staggers them; misses those behind")
{
	EnemyTuning t;
	std::vector<Enemy> es{enemyAt(0, -1.0f), enemyAt(0, 1.5f)}; // in front, behind
	tryKick(Vector3{0, 0, 0}, 0.0f, es, 1.6f, 14.0f, t);
	CHECK(es[0].velocity.z < 0.0f); // front: shoved away (-Z)
	CHECK(es[0].state == EnemyState::Stagger);
	CHECK(es[1].velocity.z == doctest::Approx(0.0)); // behind: untouched
	CHECK(es[1].state == EnemyState::Approach);
}

TEST_CASE("enemy approaches; stagger and death timers expire")
{
	EnemyTuning t;
	std::vector<Enemy> es{enemyAt(0, -10.0f)};
	PlayerTarget p = targetAt({0, 0, 0});
	step(es, p, t, 60);
	CHECK(std::fabs(es[0].position.z) < 10.0f); // moved closer

	es[0].state = EnemyState::Stagger;
	es[0].stateTimer = 0.1f;
	step(es, p, t, 12);
	CHECK(es[0].state == EnemyState::Approach);

	es[0].state = EnemyState::Dead;
	es[0].stateTimer = 0.05f;
	es[0].active = true;
	step(es, p, t, 12);
	CHECK_FALSE(es[0].active);
}

TEST_CASE("enemy in range winds up then strikes the player")
{
	EnemyTuning t;
	std::vector<Enemy> es{enemyAt(0, -1.0f)}; // inside attackRange
	float hp = 100.0f;
	PlayerTarget p = targetAt({0, 0, 0}, &hp);
	updateEnemies(es, p, t, 1.0f / 60.0f);
	CHECK(es[0].state == EnemyState::Windup); // committed
	CHECK(hp == doctest::Approx(100.0));      // no damage yet during telegraph
	step(es, p, t, (int)(t.attackWindup * 60.0f) + 2);
	CHECK(hp == doctest::Approx(100.0f - t.attackDamage));
	CHECK(es[0].state == EnemyState::Recover);
}

TEST_CASE("stepping out of reach during the windup dodges the hit")
{
	EnemyTuning t;
	std::vector<Enemy> es{enemyAt(0, -1.0f)};
	float hp = 100.0f;
	PlayerTarget p = targetAt({0, 0, 0}, &hp);
	updateEnemies(es, p, t, 1.0f / 60.0f); // -> Windup
	p.pos = {0, 0, 10.0f};                 // back away past attackReach before it lands
	step(es, p, t, (int)(t.attackWindup * 60.0f) + 2);
	CHECK(hp == doctest::Approx(100.0)); // whiffed
}

TEST_CASE("a facing shield absorbs most of a strike; a flank hit lands full")
{
	EnemyTuning t;
	{
		float hp = 100.0f;
		PlayerTarget p = targetAt({0, 0, 0}, &hp, true, 0.0f); // facing -Z, toward the enemy
		std::vector<Enemy> es{enemyAt(0, -1.0f)};
		step(es, p, t, (int)(t.attackWindup * 60.0f) + 3);
		CHECK(hp == doctest::Approx(100.0f - t.attackDamage * (1.0f - t.blockReduction)));
	}
	{
		float hp = 100.0f;
		PlayerTarget p = targetAt({0, 0, 0}, &hp, true, 0.0f); // shield up but enemy is behind (+Z)
		std::vector<Enemy> es{enemyAt(0, 1.0f)};
		step(es, p, t, (int)(t.attackWindup * 60.0f) + 3);
		CHECK(hp == doctest::Approx(100.0f - t.attackDamage)); // flank/back: unblocked
	}
}

TEST_CASE("a kick during the windup interrupts the strike")
{
	EnemyTuning t;
	std::vector<Enemy> es{enemyAt(0, -1.0f)};
	float hp = 100.0f;
	PlayerTarget p = targetAt({0, 0, 0}, &hp);
	updateEnemies(es, p, t, 1.0f / 60.0f); // -> Windup
	tryKick(p.pos, p.yaw, es, 1.6f, 14.0f, t);
	CHECK(es[0].state == EnemyState::Stagger);
	step(es, p, t, (int)(t.attackWindup * 60.0f) + 3);
	CHECK(hp == doctest::Approx(100.0)); // interrupted -> never struck
}
