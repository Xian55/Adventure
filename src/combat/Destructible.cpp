#include "combat/Destructible.h"

#include <cmath>

namespace adventure
{
	int damageProps(std::vector<Destructible>& props, std::vector<Pickup>& pickups, Vector3 center, float radius, float damage, const PropTuning& t)
	{
		int broke = 0;
		for (Destructible& p : props)
		{
			if (!p.active || p.broken)
				continue;
			const float dx = p.position.x - center.x;
			const float dz = p.position.z - center.z;
			const float dy = p.position.y - center.y;
			if (std::sqrt(dx * dx + dz * dz) > radius + p.radius) // horizontal reach
				continue;
			if (std::fabs(dy) > p.height * 0.5f + radius) // roughly the same height band
				continue;

			p.health -= damage;
			if (p.health <= 0.0f)
			{
				p.broken = true;
				p.breakTimer = t.debrisTime;
				if (p.loot != LootKind::None)
					pickups.push_back(Pickup{p.position, p.loot, true}); // container spills its loot
				++broke;
			}
		}
		return broke;
	}

	void updateProps(std::vector<Destructible>& props, const PropTuning& t, float dt)
	{
		(void)t;
		for (Destructible& p : props)
		{
			if (!p.active || !p.broken)
				continue;
			p.breakTimer -= dt;
			if (p.breakTimer <= 0.0f)
				p.active = false;
		}
	}

	void collectPickups(std::vector<Pickup>& pickups, Vector3 playerPos, float& playerHealth, float maxHealth, const PropTuning& t)
	{
		const float r2 = t.pickupRadius * t.pickupRadius;
		for (Pickup& pk : pickups)
		{
			if (!pk.active)
				continue;
			const float dx = pk.position.x - playerPos.x;
			const float dy = pk.position.y - playerPos.y;
			const float dz = pk.position.z - playerPos.z;
			if (dx * dx + dy * dy + dz * dz > r2)
				continue;

			if (pk.kind == LootKind::Health)
			{
				playerHealth += t.healAmount;
				if (playerHealth > maxHealth)
					playerHealth = maxHealth;
			}
			pk.active = false;
		}
	}
} // namespace adventure
