#pragma once
#include "combat/Enemy.h"
#include "combat/Melee.h"

#include <vector>

// Enemy AI + melee hit resolution. Pure logic (no raylib window) -> headless-testable.
namespace adventure
{
	struct EnemyTuning
	{
		float moveSpeed = 3.0f;     // approach speed
		float attackRange = 1.6f;   // stops approaching within this
		float staggerTime = 0.45f;  // frozen after a hit
		float deathTime = 1.0f;     // despawn delay after dying
		float knockbackDamp = 9.0f; // knockback velocity decay per second
	};

	// One fixed step of AI + knockback integration for every active enemy.
	void updateEnemies(std::vector<Enemy>& enemies, Vector3 playerPos, const EnemyTuning& t, float dt);

	// If the player's melee hitbox is live (and this swing hasn't hit yet), damage every enemy inside the
	// reach + arc: apply damage, knock them back, stagger (or kill). One resolution per swing.
	void resolveMeleeHits(MeleeState& melee, const WeaponDef& weapon, Vector3 playerPos, float playerYaw, std::vector<Enemy>& enemies, const EnemyTuning& t);

	// Kick: shove every enemy in a short forward cone hard (impulse velocity) and stagger them — the
	// Dark Messiah environmental-kill move (knock into hazards/off ledges). Cooldown is caller-managed.
	void tryKick(Vector3 playerPos, float playerYaw, std::vector<Enemy>& enemies, float reach, float impulse, const EnemyTuning& t);
} // namespace adventure
