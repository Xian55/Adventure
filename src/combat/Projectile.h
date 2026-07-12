#pragma once
#include "raylib.h"
#include "combat/Destructible.h"
#include "combat/Enemy.h"
#include "combat/CombatSystem.h"

#include <vector>

// Projectiles (crossbow bolts, later spells). Pure logic (no raylib window) -> headless-testable. Fired by
// the same charge/release the melee uses; world/render live in main.
namespace adventure
{
	enum ProjectileEffect
	{
		PROJ_NONE,
		PROJ_BURN, // fire: light the target on fire (DoT)
		PROJ_SLOW, // frost: chill the target
	};

	struct Projectile
	{
		Vector3 position{0, 0, 0};
		Vector3 velocity{0, 0, 0};
		Vector3 dir{0, 0, -1};        // launch direction (for rendering; stable when stuck)
		Color color{58, 48, 38, 255}; // render tint (school colour)
		float damage = 25.0f;
		float life = 3.0f; // seconds before it despawns
		int effect = PROJ_NONE;
		bool active = true;
		bool stuck = false; // embedded in a wall (stops moving, still renders)
	};

	// Integrate flight + gravity; despawn on lifetime. Pure.
	void updateProjectiles(std::vector<Projectile>& shots, float gravity, float dt);

	// Damage the first enemy each active bolt strikes (sphere test), then spend the bolt. A bolt that misses
	// enemies but hits a destructible prop damages it (and drops loot). Returns enemy kills. Pure.
	int resolveProjectileHits(std::vector<Projectile>& shots, std::vector<Enemy>& enemies, const EnemyTuning& t, std::vector<Destructible>* props = nullptr, std::vector<Pickup>* pickups = nullptr, const PropTuning* propTune = nullptr);
} // namespace adventure
