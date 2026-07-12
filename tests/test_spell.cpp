#include <doctest/doctest.h>
#include "combat/Spell.h"

using namespace adventure;

namespace
{
	Mana full()
	{
		Mana m;
		m.cur = m.max = 100.0f;
		return m;
	}
} // namespace

TEST_CASE("mana regenerates up to the cap")
{
	Mana m;
	m.cur = 50.0f;
	m.regen = 20.0f;
	updateMana(m, 1.0f);
	CHECK(m.cur == doctest::Approx(70.0));
	updateMana(m, 5.0f);
	CHECK(m.cur == doctest::Approx(100.0));
}

TEST_CASE("Telekinesis shoves front enemies and spends mana")
{
	EnemyTuning t;
	Mana m = full();
	std::vector<Enemy> es(2);
	es[0].position = {0, 0, -2.0f}; // front
	es[1].position = {0, 0, 2.0f};  // behind
	std::vector<Projectile> shots;
	float hp = 100.0f;

	CHECK(castSpell(m, spellDef(SPELL_TELEKINESIS), Vector3{0, 0, 0}, Vector3{0, 0, -1}, es, shots, hp, 100.0f, t));
	CHECK(m.cur < 100.0f);
	CHECK(es[0].velocity.z < 0.0f); // shoved away
	CHECK(es[1].velocity.z == doctest::Approx(0.0));
	CHECK(shots.empty()); // push spawns no projectile
}

TEST_CASE("a projectile spell spawns a school-tinted bolt with its effect")
{
	EnemyTuning t;
	Mana m = full();
	std::vector<Enemy> es;
	std::vector<Projectile> shots;
	float hp = 100.0f;

	CHECK(castSpell(m, spellDef(SPELL_FIREBALL), Vector3{0, 1, 0}, Vector3{0, 0, -1}, es, shots, hp, 100.0f, t));
	REQUIRE(shots.size() == 1);
	CHECK(shots[0].effect == PROJ_BURN); // fire burns
	CHECK(shots[0].velocity.z < 0.0f);   // flies forward

	castSpell(m, spellDef(SPELL_FROSTBOLT), Vector3{0, 1, 0}, Vector3{0, 0, -1}, es, shots, hp, 100.0f, t);
	REQUIRE(shots.size() == 2);
	CHECK(shots[1].effect == PROJ_SLOW); // frost slows
}

TEST_CASE("a melee spell (Quake) damages enemies in the cone")
{
	EnemyTuning t;
	Mana m = full();
	std::vector<Enemy> es(1);
	es[0].position = {0, 0, -2.0f};
	es[0].health = 50.0f;
	std::vector<Projectile> shots;
	float hp = 100.0f;

	castSpell(m, spellDef(SPELL_QUAKE), Vector3{0, 0, 0}, Vector3{0, 0, -1}, es, shots, hp, 100.0f, t);
	CHECK(es[0].health < 50.0f);
	CHECK(es[0].velocity.z < 0.0f); // knocked back
}

TEST_CASE("Heal restores the caster's health, clamped")
{
	EnemyTuning t;
	Mana m = full();
	std::vector<Enemy> es;
	std::vector<Projectile> shots;
	float hp = 40.0f;
	castSpell(m, spellDef(SPELL_HEAL), Vector3{0, 0, 0}, Vector3{0, 0, -1}, es, shots, hp, 100.0f, t);
	CHECK(hp > 40.0f);

	hp = 95.0f;
	castSpell(m, spellDef(SPELL_HEAL), Vector3{0, 0, 0}, Vector3{0, 0, -1}, es, shots, hp, 100.0f, t);
	CHECK(hp == doctest::Approx(100.0)); // clamped
}

TEST_CASE("a spell fails without enough mana")
{
	EnemyTuning t;
	Mana m;
	m.cur = 1.0f;
	std::vector<Enemy> es(1);
	es[0].position = {0, 0, -1.0f};
	std::vector<Projectile> shots;
	float hp = 100.0f;
	CHECK_FALSE(castSpell(m, spellDef(SPELL_TELEKINESIS), Vector3{0, 0, 0}, Vector3{0, 0, -1}, es, shots, hp, 100.0f, t));
	CHECK(m.cur == doctest::Approx(1.0));
}
