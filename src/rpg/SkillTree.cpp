#include "rpg/SkillTree.h"

namespace adventure
{
	namespace
	{
		SkillNode kNodes[SKILL_COUNT] = {
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

	void setSkillCost(int id, int rank, int cost)
	{
		if (id >= 0 && id < SKILL_COUNT && rank >= 0 && rank < 3)
			kNodes[id].cost[rank] = cost;
	}

	Stats deriveStats(const SkillState& s, const SkillTuning& t)
	{
		Stats st;
		st.maxHealthBonus = s.rank[SKILL_TOUGHNESS] * t.healthPerRank;
		st.damageMul = 1.0f + s.rank[SKILL_POWER] * t.damagePerRank;
		st.rageBuildMul = 1.0f + s.rank[SKILL_ADRENALINE] * t.rageBonus;
		st.moveSpeedMul = 1.0f + s.rank[SKILL_ENDURANCE] * t.speedPerRank;
		st.lockpick = s.rank[SKILL_LOCKPICK] > 0;
		return st;
	}
} // namespace adventure
