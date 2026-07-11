#pragma once

// RPG skill trees (Dark Messiah–style, lightweight cut). Spend earned points on ranked nodes across three
// trees; nodes derive the player's Stats and gate abilities. Pure logic (no raylib) -> headless-testable.
namespace adventure
{
	enum SkillId
	{
		SKILL_TOUGHNESS,  // Combat: +max health per rank
		SKILL_POWER,      // Combat: +melee damage per rank
		SKILL_ADRENALINE, // Combat: rage builds faster
		SKILL_ENDURANCE,  // Body: +move speed per rank
		SKILL_LOCKPICK,   // Utility: open locked chests/doors without a key
		SKILL_COUNT,
	};

	struct SkillNode
	{
		const char* name;
		const char* tree; // "Combat" / "Body" / "Utility"
		int maxRank;
		int cost[3];    // point cost to reach rank i+1 (indexed by current rank)
		int prereq;     // SkillId that must be unlocked first, or -1
		int prereqRank; // required rank of the prerequisite
	};

	const SkillNode& skillNode(int id);

	struct SkillState
	{
		int points = 0;             // unspent skill points
		int rank[SKILL_COUNT] = {}; // current rank per node
	};

	int skillCost(int id, int currentRank);      // points to reach the next rank, or -1 if maxed
	bool canUnlock(const SkillState& s, int id); // enough points + prereq met + not maxed
	bool unlockSkill(SkillState& s, int id);     // spend points + rank up; false if not allowed

	// Effective player stats derived from the unlocked nodes.
	struct Stats
	{
		float maxHealthBonus = 0.0f;
		float damageMul = 1.0f;
		float moveSpeedMul = 1.0f;
		float rageBuildMul = 1.0f;
		bool lockpick = false;
	};
	Stats deriveStats(const SkillState& s);
} // namespace adventure
