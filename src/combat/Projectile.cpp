#include "combat/Projectile.h"

#include <cmath>

namespace adventure
{
	void updateProjectiles(std::vector<Projectile>& shots, float gravity, float dt)
	{
		for (Projectile& p : shots)
		{
			if (!p.active)
				continue;
			if (!p.stuck) // a stuck bolt stays put but still ages out
			{
				p.velocity.y -= gravity * dt; // slight drop over distance
				p.position.x += p.velocity.x * dt;
				p.position.y += p.velocity.y * dt;
				p.position.z += p.velocity.z * dt;
			}
			p.life -= dt;
			if (p.life <= 0.0f)
				p.active = false;
		}
	}

	int resolveProjectileHits(std::vector<Projectile>& shots, std::vector<Enemy>& enemies, const EnemyTuning& t, std::vector<Destructible>* props, std::vector<Pickup>* pickups, const PropTuning* propTune)
	{
		int kills = 0;
		for (Projectile& p : shots)
		{
			if (!p.active || p.stuck)
				continue;
			for (Enemy& e : enemies)
			{
				if (!e.active || e.state == EnemyState::Dead)
					continue;
				const float dx = e.position.x - p.position.x;
				const float dy = e.position.y - p.position.y;
				const float dz = e.position.z - p.position.z;
				const float r = e.radius + 0.15f; // bolt radius
				if (dx * dx + dy * dy + dz * dz > r * r)
					continue;

				e.health -= p.damage;
				if (p.effect == PROJ_BURN)
					e.burn = 3.0f;
				else if (p.effect == PROJ_SLOW)
					e.slow = 2.5f;
				if (e.health <= 0.0f)
				{
					e.state = EnemyState::Dead;
					e.stateTimer = t.deathTime;
					++kills;
				}
				else
				{
					e.state = EnemyState::Stagger;
					e.stateTimer = t.staggerTime;
				}
				p.active = false; // the bolt is spent
				break;
			}

			if (p.active && props && pickups && propTune) // missed enemies — hit a prop?
				for (Destructible& pr : *props)
				{
					if (!pr.active || pr.broken)
						continue;
					const float dx = pr.position.x - p.position.x;
					const float dz = pr.position.z - p.position.z;
					const float dy = pr.position.y - p.position.y;
					if (std::sqrt(dx * dx + dz * dz) > pr.radius + 0.15f || std::fabs(dy) > pr.height * 0.5f + 0.15f)
						continue;
					pr.health -= p.damage;
					if (pr.health <= 0.0f)
					{
						pr.broken = true;
						pr.breakTimer = propTune->debrisTime;
						if (pr.dropItem != kItemNone)
							pickups->push_back(Pickup{pr.position, pr.dropItem, true, 1});
					}
					p.active = false; // spent on the prop
					break;
				}
		}
		return kills;
	}
} // namespace adventure
