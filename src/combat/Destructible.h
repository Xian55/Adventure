#pragma once
#include "raylib.h"
#include "items/Item.h"
#include "items/Pickup.h"

#include <vector>

// Destructible props (barrels / kegs / crates) — smashable containers that may drop an item. Pure logic
// (no raylib window) -> headless-testable. Solid via resolveActorProps; broken rubble is walk-through.
namespace adventure
{
	enum class PropKind
	{
		Barrel,
		Crate,
		Keg,
	};

	struct Destructible
	{
		Vector3 position{0, 0, 0}; // AABB center, engine space
		float radius = 0.35f;
		float height = 1.0f;
		float health = 30.0f;
		float maxHealth = 30.0f;
		PropKind kind = PropKind::Barrel;
		int dropItem = kItemNone; // item spawned as a Pickup when broken (0 = nothing)
		bool active = true;       // false once the debris has cleared
		bool broken = false;      // shattered; showing debris
		float breakTimer = 0.0f;  // debris lifetime remaining
	};

	struct PropTuning
	{
		float debrisTime = 0.5f;   // how long debris shows after a break
		float pickupRadius = 1.0f; // player collect distance
	};

	// Damage every intact prop within `radius` of `center` (a melee hitbox point, a kick, or a blast).
	// Breaking a prop with a dropItem spawns a Pickup. Returns the number newly broken. Pure.
	int damageProps(std::vector<Destructible>& props, std::vector<Pickup>& pickups, Vector3 center, float radius, float damage, const PropTuning& t);

	// Advance debris timers; despawn props once their debris has cleared. Pure.
	void updateProps(std::vector<Destructible>& props, const PropTuning& t, float dt);

	// Push an actor (vertical cylinder, `radius`/`height` about center `pos`) horizontally out of every intact
	// prop it overlaps, so props block movement. Broken/inactive props don't collide. Pure. Linear over props
	// (a broadphase is a later optimization). `pos` is updated in place.
	void resolveActorProps(Vector3& pos, float radius, float height, const std::vector<Destructible>& props);
} // namespace adventure
