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
				if (p.dropItem != kItemNone)
					pickups.push_back(Pickup{p.position, p.dropItem, true}); // container spills its loot
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

	void resolveActorProps(Vector3& pos, float radius, float height, const std::vector<Destructible>& props)
	{
		for (const Destructible& p : props)
		{
			if (!p.active || p.broken)
				continue;
			// Vertical bands must overlap for a horizontal block (step over/under otherwise).
			if (pos.y - height * 0.5f >= p.position.y + p.height * 0.5f || pos.y + height * 0.5f <= p.position.y - p.height * 0.5f)
				continue;

			const float dx = pos.x - p.position.x;
			const float dz = pos.z - p.position.z;
			const float minD = radius + p.radius;
			const float d = std::sqrt(dx * dx + dz * dz);
			if (d >= minD)
				continue;

			if (d > 0.0001f)
			{
				const float push = minD - d;
				pos.x += (dx / d) * push;
				pos.z += (dz / d) * push;
			}
			else
			{
				pos.x += minD; // dead-centre: shove out along +x
			}
		}
	}
} // namespace adventure
