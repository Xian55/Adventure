#pragma once
#include "raylib.h"
#include "combat/Enemy.h"
#include "combat/CombatSystem.h"

#include <vector>

// Projectiles (crossbow bolts, later spells). Pure logic (no raylib window) -> headless-testable. Fired by
// the same charge/release the melee uses; world/render live in main.
namespace adventure
{
	struct Projectile
	{
		Vector3 position{0, 0, 0};
		Vector3 velocity{0, 0, 0};
		Vector3 dir{0, 0, -1}; // launch direction (for rendering; stable when stuck)
		float damage = 25.0f;
		float life = 3.0f; // seconds before it despawns
		bool active = true;
		bool stuck = false; // embedded in a wall (stops moving, still renders)
	};

	// Integrate flight + gravity; despawn on lifetime. Pure.
	void updateProjectiles(std::vector<Projectile>& shots, float gravity, float dt);

	// Damage the first enemy each active bolt strikes (sphere test), then despawn the bolt. Returns kills.
	int resolveProjectileHits(std::vector<Projectile>& shots, std::vector<Enemy>& enemies, const EnemyTuning& t);
} // namespace adventure
