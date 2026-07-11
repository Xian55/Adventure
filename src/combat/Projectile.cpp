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

	int resolveProjectileHits(std::vector<Projectile>& shots, std::vector<Enemy>& enemies, const EnemyTuning& t)
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
		}
		return kills;
	}
} // namespace adventure
