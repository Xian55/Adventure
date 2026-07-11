#pragma once
#include "raylib.h"
#include "combat/Enemy.h"
#include "combat/CombatSystem.h"

#include <vector>

// Spells + mana. Data-driven (defs from scripts/spells.lua). Pure logic (no raylib window) ->
// headless-testable. The first spell is Telekinesis (a ranged force-push — ties into the kick/physics:
// shove enemies into hazards or off ledges).
namespace adventure
{
	enum SpellId
	{
		SPELL_PUSH, // Telekinesis: forward-cone shove
		SPELL_COUNT,
	};

	struct SpellDef
	{
		const char* name = "Spell";
		float manaCost = 25.0f;
		float power = 18.0f;  // impulse applied
		float range = 6.0f;   // reach
		float coneCos = 0.4f; // front-cone threshold (dot); ~66 deg half-angle
	};
	const SpellDef& spellDef(int id);
	void setSpellDef(int id, float manaCost, float power, float range);

	struct Mana
	{
		float cur = 100.0f;
		float max = 100.0f;
		float regen = 12.0f; // per second
	};
	void updateMana(Mana& m, float dt);

	// Cast Telekinesis: if enough mana, spend it and shove every enemy in the forward cone within range
	// (impulse into their velocity + stagger). Returns true if cast. Pure.
	bool castPush(Mana& m, const SpellDef& s, Vector3 origin, float yaw, std::vector<Enemy>& enemies, const EnemyTuning& t);
} // namespace adventure
