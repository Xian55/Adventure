#pragma once
#include "combat/Enemy.h"
#include "combat/Melee.h"
#include "world/MapTypes.h"

#include <vector>

// Enemy AI + melee hit resolution. Pure logic (no raylib window) -> headless-testable.
namespace adventure
{
	struct EnemyTuning
	{
		float moveSpeed = 3.0f;      // approach speed
		float attackRange = 1.6f;    // stops approaching / starts a swing within this
		float attackWindup = 0.5f;   // telegraph before the hit lands (dodge/kick window)
		float attackRecover = 0.6f;  // cooldown after swinging
		float attackReach = 1.9f;    // hit connects if the player is within this at strike time
		float attackDamage = 12.0f;  // damage per landed hit
		float staggerTime = 0.45f;   // frozen after being hit
		float deathTime = 1.0f;      // despawn delay after dying
		float knockbackDamp = 9.0f;  // knockback velocity decay per second
		float blockArc = 1.2f;       // front half-cone the raised shield covers (radians)
		float blockReduction = 0.8f; // fraction of damage absorbed by a facing block
	};

	// What an enemy needs to strike the player back. `health` is written on a landed hit.
	struct PlayerTarget
	{
		Vector3 pos{0, 0, 0};
		float yaw = 0.0f; // facing; 0 => -Z
		bool shieldRaised = false;
		float* health = nullptr; // decremented when a hit lands (null = invulnerable)
	};

	// One fixed step of AI + knockback integration for every active enemy. Enemies approach, wind up,
	// and strike the player (reduced/negated by a facing shield); the target's health is written through.
	void updateEnemies(std::vector<Enemy>& enemies, PlayerTarget& player, const EnemyTuning& t, float dt);

	// Enemies touched by a single swing resolution (feeds the rage meter).
	struct MeleeHitResult
	{
		int hits = 0;
		int kills = 0;
	};

	// If the player's melee hitbox is live (and this swing hasn't hit yet), damage every enemy inside the
	// reach + arc: apply damage (× damageMul), knock them back, stagger (or kill). One resolution per swing.
	MeleeHitResult resolveMeleeHits(MeleeState& melee, const WeaponDef& weapon, Vector3 playerPos, float playerYaw, std::vector<Enemy>& enemies, const EnemyTuning& t, float damageMul = 1.0f);

	// Kick: shove every enemy in a short forward cone hard (impulse velocity) and stagger them — the
	// Dark Messiah environmental-kill move (knock into hazards/off ledges). Cooldown is caller-managed.
	void tryKick(Vector3 playerPos, float playerYaw, std::vector<Enemy>& enemies, float reach, float impulse, const EnemyTuning& t);

	// Damage every enemy standing in a hazard volume (kick them in for an environmental kill). dt-scaled.
	void applyHazards(std::vector<Enemy>& enemies, const std::vector<world::Hazard>& hazards, const EnemyTuning& t, float dt);
} // namespace adventure
