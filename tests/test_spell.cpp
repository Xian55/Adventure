#include <doctest/doctest.h>
#include "combat/Spell.h"

using namespace adventure;

TEST_CASE("mana regenerates up to the cap")
{
	Mana m;
	m.cur = 50.0f;
	m.max = 100.0f;
	m.regen = 20.0f;
	updateMana(m, 1.0f);
	CHECK(m.cur == doctest::Approx(70.0));
	updateMana(m, 5.0f); // would overshoot
	CHECK(m.cur == doctest::Approx(100.0));
}

TEST_CASE("Telekinesis shoves enemies in the cone and spends mana")
{
	EnemyTuning t;
	SpellDef s = spellDef(SPELL_PUSH);
	Mana m;
	m.cur = 100.0f;

	std::vector<Enemy> es(2);
	es[0].position = {0, 0, -2.0f}; // in front, in range
	es[1].position = {0, 0, 2.0f};  // behind

	CHECK(castPush(m, s, Vector3{0, 0, 0}, 0.0f, es, t));
	CHECK(m.cur == doctest::Approx(100.0f - s.manaCost));
	CHECK(es[0].velocity.z < 0.0f); // shoved away (-Z)
	CHECK(es[0].state == EnemyState::Stagger);
	CHECK(es[1].velocity.z == doctest::Approx(0.0)); // behind: untouched
}

TEST_CASE("casting fails without enough mana")
{
	EnemyTuning t;
	SpellDef s = spellDef(SPELL_PUSH);
	Mana m;
	m.cur = 5.0f; // < cost
	std::vector<Enemy> es(1);
	es[0].position = {0, 0, -1.0f};
	CHECK_FALSE(castPush(m, s, Vector3{0, 0, 0}, 0.0f, es, t));
	CHECK(m.cur == doctest::Approx(5.0)); // unchanged
	CHECK(es[0].velocity.z == doctest::Approx(0.0));
}

TEST_CASE("an out-of-range enemy is not pushed")
{
	EnemyTuning t;
	SpellDef s = spellDef(SPELL_PUSH); // range 6
	Mana m;
	m.cur = 100.0f;
	std::vector<Enemy> es(1);
	es[0].position = {0, 0, -9.0f}; // beyond range
	CHECK(castPush(m, s, Vector3{0, 0, 0}, 0.0f, es, t));
	CHECK(es[0].velocity.z == doctest::Approx(0.0));
}
