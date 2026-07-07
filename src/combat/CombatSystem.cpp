#include "combat/CombatSystem.h"

#include <cmath>

namespace adventure
{
	void updateEnemies(std::vector<Enemy>& enemies, Vector3 playerPos, const EnemyTuning& t, float dt)
	{
		for (Enemy& e : enemies)
		{
			if (!e.active)
				continue;

			// Knockback integration (horizontal), with decay.
			e.position.x += e.velocity.x * dt;
			e.position.z += e.velocity.z * dt;
			const float damp = fmaxf(0.0f, 1.0f - t.knockbackDamp * dt);
			e.velocity.x *= damp;
			e.velocity.z *= damp;

			switch (e.state)
			{
			case EnemyState::Approach:
			{
				float tx = playerPos.x - e.position.x;
				float tz = playerPos.z - e.position.z;
				float d = std::sqrt(tx * tx + tz * tz);
				if (d > t.attackRange && d > 0.0001f)
				{
					e.position.x += (tx / d) * t.moveSpeed * dt;
					e.position.z += (tz / d) * t.moveSpeed * dt;
					e.yaw = std::atan2(tx / d, -tz / d); // face the player (yaw 0 => -Z)
				}
				break;
			}
			case EnemyState::Stagger:
				e.stateTimer -= dt;
				if (e.stateTimer <= 0.0f)
					e.state = EnemyState::Approach;
				break;
			case EnemyState::Dead:
				e.stateTimer -= dt;
				if (e.stateTimer <= 0.0f)
					e.active = false;
				break;
			}
		}
	}

	void resolveMeleeHits(MeleeState& melee, const WeaponDef& weapon, Vector3 playerPos, float playerYaw, std::vector<Enemy>& enemies, const EnemyTuning& t)
	{
		if (!hitboxActive(melee) || melee.hitThisSwing)
			return;

		const float fx = std::sin(playerYaw);
		const float fz = -std::cos(playerYaw);
		const float cosHalf = std::cos(weapon.arc * 0.5f);
		bool anyHit = false;

		for (Enemy& e : enemies)
		{
			if (!e.active || e.state == EnemyState::Dead)
				continue;

			const float tx = e.position.x - playerPos.x;
			const float tz = e.position.z - playerPos.z;
			const float d = std::sqrt(tx * tx + tz * tz);
			if (d > weapon.reach + e.radius)
				continue;
			if (d > 0.0001f && (fx * tx + fz * tz) / d < cosHalf) // outside the swing arc
				continue;

			e.health -= weapon.damage;
			e.velocity.x += fx * weapon.knockback;
			e.velocity.z += fz * weapon.knockback;
			if (e.health <= 0.0f)
			{
				e.state = EnemyState::Dead;
				e.stateTimer = t.deathTime;
			}
			else
			{
				e.state = EnemyState::Stagger;
				e.stateTimer = t.staggerTime;
			}
			anyHit = true;
		}

		if (anyHit)
			melee.hitThisSwing = true; // this swing has connected; don't hit again
	}
} // namespace adventure
