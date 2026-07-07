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

		// Aim shifts with the swing direction (left/right slashes reach to that side); charge scales power.
		float aimYaw = playerYaw;
		if (melee.resolved == SwingDir::Left)
			aimYaw -= weapon.arc * 0.35f;
		else if (melee.resolved == SwingDir::Right)
			aimYaw += weapon.arc * 0.35f;
		const float charge = chargeFraction(melee, weapon);
		const float dmg = weapon.damage * (1.0f + charge * weapon.chargeDamageMul);
		const float kb = weapon.knockback * (1.0f + charge * 0.5f);

		const float fx = std::sin(aimYaw);
		const float fz = -std::cos(aimYaw);
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

			e.health -= dmg;
			e.velocity.x += fx * kb;
			e.velocity.z += fz * kb;
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

	void tryKick(Vector3 playerPos, float playerYaw, std::vector<Enemy>& enemies, float reach, float impulse, const EnemyTuning& t)
	{
		const float fx = std::sin(playerYaw);
		const float fz = -std::cos(playerYaw);
		const float cosHalf = std::cos(0.9f); // ~50deg half-cone in front

		for (Enemy& e : enemies)
		{
			if (!e.active || e.state == EnemyState::Dead)
				continue;
			const float tx = e.position.x - playerPos.x;
			const float tz = e.position.z - playerPos.z;
			const float d = std::sqrt(tx * tx + tz * tz);
			if (d > reach + e.radius)
				continue;
			if (d > 0.0001f && (fx * tx + fz * tz) / d < cosHalf)
				continue;

			e.velocity.x += fx * impulse;
			e.velocity.z += fz * impulse;
			e.state = EnemyState::Stagger;
			e.stateTimer = t.staggerTime;
		}
	}
} // namespace adventure
