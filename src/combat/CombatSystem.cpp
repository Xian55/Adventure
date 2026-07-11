#include "combat/CombatSystem.h"
#include "world/BrushGeometry.h"

#include <cmath>

namespace adventure
{
	// Apply one enemy strike to the player: a facing raised shield absorbs most of it; flank/back hits land full.
	static void strikePlayer(const Enemy& e, PlayerTarget& player, const EnemyTuning& t)
	{
		if (!player.health)
			return;
		float dmg = t.attackDamage;
		const float dx = e.position.x - player.pos.x;
		const float dz = e.position.z - player.pos.z;
		const float d = std::sqrt(dx * dx + dz * dz);
		if (player.shieldRaised && d > 0.0001f)
		{
			const float pfx = std::sin(player.yaw);
			const float pfz = -std::cos(player.yaw);
			if ((pfx * dx + pfz * dz) / d >= std::cos(t.blockArc)) // enemy inside the shield's front cone
				dmg *= (1.0f - t.blockReduction);
		}
		*player.health -= dmg;
	}

	void updateEnemies(std::vector<Enemy>& enemies, PlayerTarget& player, const EnemyTuning& t, float dt)
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

			const float tx = player.pos.x - e.position.x;
			const float tz = player.pos.z - e.position.z;
			const float d = std::sqrt(tx * tx + tz * tz);
			if (e.state != EnemyState::Dead && d > 0.0001f)
				e.yaw = std::atan2(tx / d, -tz / d); // face the player (yaw 0 => -Z)

			switch (e.state)
			{
			case EnemyState::Approach:
				if (d <= t.attackRange)
				{
					e.state = EnemyState::Windup; // in range: commit to a swing
					e.stateTimer = t.attackWindup;
				}
				else if (d > 0.0001f)
				{
					e.position.x += (tx / d) * t.moveSpeed * dt;
					e.position.z += (tz / d) * t.moveSpeed * dt;
				}
				break;
			case EnemyState::Windup:
				e.stateTimer -= dt;
				if (e.stateTimer <= 0.0f)
				{
					if (d <= t.attackReach) // player still in range at the strike moment -> it lands
						strikePlayer(e, player, t);
					e.state = EnemyState::Recover;
					e.stateTimer = t.attackRecover;
				}
				break;
			case EnemyState::Recover:
				e.stateTimer -= dt;
				if (e.stateTimer <= 0.0f)
					e.state = EnemyState::Approach;
				break;
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

	MeleeHitResult resolveMeleeHits(MeleeState& melee, const WeaponDef& weapon, Vector3 playerPos, float playerYaw, std::vector<Enemy>& enemies, const EnemyTuning& t, float damageMul, std::vector<Destructible>* props, std::vector<Pickup>* pickups, const PropTuning* propTune)
	{
		MeleeHitResult result;
		if (!hitboxActive(melee) || melee.hitThisSwing)
			return result;

		// Aim shifts with the swing direction (left/right slashes reach to that side); charge scales power.
		float aimYaw = playerYaw;
		if (melee.resolved == SwingDir::Left)
			aimYaw -= weapon.arc * 0.35f;
		else if (melee.resolved == SwingDir::Right)
			aimYaw += weapon.arc * 0.35f;
		const float charge = chargeFraction(melee, weapon);
		const float dmg = weapon.damage * (1.0f + charge * weapon.chargeDamageMul) * damageMul;
		const float kb = weapon.knockback * (1.0f + charge * 0.5f);

		const float fx = std::sin(aimYaw);
		const float fz = -std::cos(aimYaw);
		const float cosHalf = std::cos(weapon.arc * 0.5f);

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
				++result.kills;
			}
			else
			{
				e.state = EnemyState::Stagger;
				e.stateTimer = t.staggerTime;
			}
			++result.hits;
		}

		bool propHit = false;
		if (props && pickups && propTune) // the same swing smashes props in its arc
		{
			for (Destructible& p : *props)
			{
				if (!p.active || p.broken)
					continue;
				const float tx = p.position.x - playerPos.x;
				const float tz = p.position.z - playerPos.z;
				const float d = std::sqrt(tx * tx + tz * tz);
				if (d > weapon.reach + p.radius)
					continue;
				if (d > 0.0001f && (fx * tx + fz * tz) / d < cosHalf)
					continue;

				p.health -= dmg;
				propHit = true;
				if (p.health <= 0.0f)
				{
					p.broken = true;
					p.breakTimer = propTune->debrisTime;
					if (p.loot != LootKind::None)
						pickups->push_back(Pickup{p.position, p.loot, true});
				}
			}
		}

		if (result.hits > 0 || propHit)
			melee.hitThisSwing = true; // this swing has connected; don't hit again
		return result;
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

	void applyHazards(std::vector<Enemy>& enemies, const std::vector<world::Hazard>& hazards, const EnemyTuning& t, float dt)
	{
		if (hazards.empty())
			return;
		for (Enemy& e : enemies)
		{
			if (!e.active || e.state == EnemyState::Dead)
				continue;
			const Vector3 feet = {e.position.x, e.position.y - e.height * 0.5f, e.position.z};
			const float dps = world::hazardDamageAt(hazards, feet);
			if (dps <= 0.0f)
				continue;
			e.health -= dps * dt;
			if (e.health <= 0.0f)
			{
				e.state = EnemyState::Dead;
				e.stateTimer = t.deathTime;
			}
		}
	}
} // namespace adventure
