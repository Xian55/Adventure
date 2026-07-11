#include "rpg/SkillTree.h"

namespace adventure
{
	namespace
	{
		const SkillNode kNodes[SKILL_COUNT] = {
		    // name           tree        maxRank  cost{r0,r1,r2}   prereq          prereqRank
		    {"Toughness", "Combat", 3, {1, 2, 4}, -1, 0},
		    {"Power", "Combat", 2, {2, 4, 0}, SKILL_TOUGHNESS, 1},
		    {"Adrenaline", "Combat", 1, {6, 0, 0}, SKILL_POWER, 1},
		    {"Endurance", "Body", 2, {1, 3, 0}, -1, 0},
		    {"Lockpicking", "Utility", 1, {4, 0, 0}, -1, 0},
		};
	} // namespace

	const SkillNode& skillNode(int id)
	{
		return kNodes[(id >= 0 && id < SKILL_COUNT) ? id : 0];
	}

	int skillCost(int id, int currentRank)
	{
		const SkillNode& n = skillNode(id);
		if (currentRank < 0 || currentRank >= n.maxRank)
			return -1;
		return n.cost[currentRank];
	}

	bool canUnlock(const SkillState& s, int id)
	{
		const SkillNode& n = skillNode(id);
		if (s.rank[id] >= n.maxRank)
			return false;
		if (n.prereq >= 0 && s.rank[n.prereq] < n.prereqRank)
			return false;
		return s.points >= n.cost[s.rank[id]];
	}

	bool unlockSkill(SkillState& s, int id)
	{
		if (!canUnlock(s, id))
			return false;
		s.points -= skillNode(id).cost[s.rank[id]];
		s.rank[id] += 1;
		return true;
	}

	Stats deriveStats(const SkillState& s)
	{
		Stats st;
		st.maxHealthBonus = s.rank[SKILL_TOUGHNESS] * 20.0f;
		st.damageMul = 1.0f + s.rank[SKILL_POWER] * 0.15f;
		st.rageBuildMul = 1.0f + s.rank[SKILL_ADRENALINE] * 0.5f;
		st.moveSpeedMul = 1.0f + s.rank[SKILL_ENDURANCE] * 0.08f;
		st.lockpick = s.rank[SKILL_LOCKPICK] > 0;
		return st;
	}
} // namespace adventure
