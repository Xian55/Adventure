#include <doctest/doctest.h>
#include "rpg/SkillTree.h"

using namespace adventure;

TEST_CASE("unlocking spends points and ranks up; blocked when broke or maxed")
{
	SkillState s;
	CHECK_FALSE(canUnlock(s, SKILL_TOUGHNESS)); // 0 points
	s.points = 10;

	CHECK(canUnlock(s, SKILL_TOUGHNESS));
	CHECK(unlockSkill(s, SKILL_TOUGHNESS)); // cost 1
	CHECK(s.rank[SKILL_TOUGHNESS] == 1);
	CHECK(s.points == 9);

	unlockSkill(s, SKILL_TOUGHNESS); // cost 2 -> rank 2, points 7
	unlockSkill(s, SKILL_TOUGHNESS); // cost 4 -> rank 3, points 3
	CHECK(s.rank[SKILL_TOUGHNESS] == 3);
	CHECK(s.points == 3);
	CHECK_FALSE(canUnlock(s, SKILL_TOUGHNESS)); // maxed
	CHECK(skillCost(SKILL_TOUGHNESS, 3) == -1);
}

TEST_CASE("prerequisites gate a node")
{
	SkillState s;
	s.points = 20;
	CHECK_FALSE(canUnlock(s, SKILL_POWER)); // needs Toughness rank 1

	unlockSkill(s, SKILL_TOUGHNESS);
	CHECK(canUnlock(s, SKILL_POWER)); // prereq met
	CHECK(unlockSkill(s, SKILL_POWER));

	CHECK(canUnlock(s, SKILL_ADRENALINE)); // needs Power rank 1 — now met, and affordable
}

TEST_CASE("not enough points blocks an otherwise-valid unlock")
{
	SkillState s;
	s.points = 1;
	unlockSkill(s, SKILL_TOUGHNESS); // cost 1 -> points 0
	CHECK(s.rank[SKILL_TOUGHNESS] == 1);
	CHECK_FALSE(canUnlock(s, SKILL_TOUGHNESS)); // rank2 costs 2, only 0 points
}

TEST_CASE("derived stats reflect unlocked ranks")
{
	SkillState s;
	Stats base = deriveStats(s);
	CHECK(base.maxHealthBonus == doctest::Approx(0.0));
	CHECK(base.damageMul == doctest::Approx(1.0));
	CHECK_FALSE(base.lockpick);

	s.rank[SKILL_TOUGHNESS] = 3;
	s.rank[SKILL_POWER] = 2;
	s.rank[SKILL_ENDURANCE] = 1;
	s.rank[SKILL_LOCKPICK] = 1;
	Stats st = deriveStats(s);
	CHECK(st.maxHealthBonus == doctest::Approx(60.0)); // 3 * 20
	CHECK(st.damageMul == doctest::Approx(1.30));      // 1 + 2 * 0.15
	CHECK(st.moveSpeedMul == doctest::Approx(1.08));   // 1 + 1 * 0.08
	CHECK(st.lockpick);
}
