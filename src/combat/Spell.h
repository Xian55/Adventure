#pragma once
#include "raylib.h"
#include "combat/Enemy.h"
#include "combat/CombatSystem.h"
#include "combat/Projectile.h"

#include <vector>

// Spellbook + mana. Data-driven (defs from scripts/spells.lua). Pure logic (no raylib window) ->
// headless-testable. Spells have a school (Force/Fire/Frost/Lightning/Earth/Holy) and a cast kind
// (Push / Projectile / Melee cone / Heal). Fire burns, Frost slows — the school flavours the effect.
namespace adventure
{
	enum SpellSchool
	{
		School_Force,
		School_Fire,
		School_Frost,
		School_Lightning,
		School_Earth,
		School_Holy,
	};

	enum SpellKind
	{
		Cast_Push,       // Telekinesis: shove enemies away
		Cast_Projectile, // a flying bolt
		Cast_Melee,      // a short cone hit
		Cast_Heal,       // restore the caster's health
	};

	enum SpellId
	{
		SPELL_TELEKINESIS,
		SPELL_FIREBALL,
		SPELL_FROSTBOLT,
		SPELL_LIGHTNING,
		SPELL_QUAKE,
		SPELL_HEAL,
		SPELL_COUNT,
	};

	struct SpellDef
	{
		const char* name;
		int school;
		int kind;
		float manaCost;
		float power;           // damage / push impulse / heal amount
		float range;           // reach (push + melee)
		float projectileSpeed; // projectile kind
	};

	const SpellDef& spellDef(int id);
	void setSpellDef(int id, float manaCost, float power, float range);
	Color schoolColor(int school);
	const char* schoolName(int school);

	struct Mana
	{
		float cur = 100.0f;
		float max = 100.0f;
		float regen = 12.0f;
	};
	void updateMana(Mana& m, float dt);

	// Cast a spell, dispatching by kind: Push shoves / Melee damages enemies in a cone; Projectile spawns
	// a bolt into `shots`; Heal restores playerHealth. `aim` is the unit look direction. Spends mana;
	// returns true if cast. Pure.
	bool castSpell(Mana& m, const SpellDef& s, Vector3 eye, Vector3 aim, std::vector<Enemy>& enemies, std::vector<Projectile>& shots, float& playerHealth, float maxHealth, const EnemyTuning& t);
} // namespace adventure
