#pragma once
#include "raylib.h"

#include <vector>

// Destructible props (barrels / kegs / crates) — smashable containers that may drop loot. Pure logic
// (no raylib window) -> headless-testable. Not solid for now (walk-through); prop-vs-actor collision later.
namespace adventure
{
	enum class PropKind
	{
		Barrel,
		Crate,
		Keg,
	};

	enum class LootKind
	{
		None,
		Health,
	};

	struct Destructible
	{
		Vector3 position{0, 0, 0}; // AABB center, engine space
		float radius = 0.35f;
		float height = 1.0f;
		float health = 30.0f;
		float maxHealth = 30.0f;
		PropKind kind = PropKind::Barrel;
		LootKind loot = LootKind::None;
		bool active = true;      // false once the debris has cleared
		bool broken = false;     // shattered; showing debris
		float breakTimer = 0.0f; // debris lifetime remaining
	};

	struct Pickup
	{
		Vector3 position{0, 0, 0};
		LootKind kind = LootKind::Health;
		bool active = true;
	};

	struct PropTuning
	{
		float debrisTime = 0.5f;   // how long debris shows after a break
		float healAmount = 25.0f;  // health orb value
		float pickupRadius = 1.0f; // player collect distance
	};

	// Damage every intact prop within `radius` of `center` (a melee hitbox point, a kick, or a blast).
	// Breaking a prop spawns its loot pickup. Returns the number newly broken. Pure.
	int damageProps(std::vector<Destructible>& props, std::vector<Pickup>& pickups, Vector3 center, float radius, float damage, const PropTuning& t);

	// Advance debris timers; despawn props once their debris has cleared. Pure.
	void updateProps(std::vector<Destructible>& props, const PropTuning& t, float dt);

	// Collect any pickup the player is standing on (heal, clamped to maxHealth). Pure.
	void collectPickups(std::vector<Pickup>& pickups, Vector3 playerPos, float& playerHealth, float maxHealth, const PropTuning& t);
} // namespace adventure
