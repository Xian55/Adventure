#include "combat/Spell.h"

#include <cmath>

namespace adventure
{
	namespace
	{
		SpellDef kSpells[SPELL_COUNT] = {
		    // name          school            kind             mana  power range projSpeed
		    {"Telekinesis", School_Force, Cast_Push, 25.0f, 18.0f, 6.0f, 0.0f},
		    {"Fireball", School_Fire, Cast_Projectile, 20.0f, 30.0f, 0.0f, 26.0f},
		    {"Frostbolt", School_Frost, Cast_Projectile, 18.0f, 22.0f, 0.0f, 24.0f},
		    {"Lightning", School_Lightning, Cast_Projectile, 30.0f, 45.0f, 0.0f, 55.0f},
		    {"Quake", School_Earth, Cast_Melee, 35.0f, 35.0f, 3.0f, 0.0f},
		    {"Heal", School_Holy, Cast_Heal, 40.0f, 40.0f, 0.0f, 0.0f},
		};

		int schoolEffect(int school)
		{
			if (school == School_Fire)
				return PROJ_BURN;
			if (school == School_Frost)
				return PROJ_SLOW;
			return PROJ_NONE;
		}
	} // namespace

	const SpellDef& spellDef(int id)
	{
		return kSpells[(id >= 0 && id < SPELL_COUNT) ? id : 0];
	}

	void setSpellDef(int id, float manaCost, float power, float range)
	{
		if (id >= 0 && id < SPELL_COUNT)
		{
			kSpells[id].manaCost = manaCost;
			kSpells[id].power = power;
			kSpells[id].range = range;
		}
	}

	Color schoolColor(int school)
	{
		switch (school)
		{
		case School_Fire:
			return Color{235, 110, 30, 255};
		case School_Frost:
			return Color{120, 200, 235, 255};
		case School_Lightning:
			return Color{235, 235, 130, 255};
		case School_Earth:
			return Color{150, 120, 80, 255};
		case School_Holy:
			return Color{240, 230, 150, 255};
		case School_Force:
		default:
			return Color{200, 160, 235, 255};
		}
	}

	const char* schoolName(int school)
	{
		switch (school)
		{
		case School_Fire:
			return "Fire";
		case School_Frost:
			return "Frost";
		case School_Lightning:
			return "Lightning";
		case School_Earth:
			return "Earth";
		case School_Holy:
			return "Holy";
		case School_Force:
		default:
			return "Force";
		}
	}

	void updateMana(Mana& m, float dt)
	{
		m.cur += m.regen * dt;
		if (m.cur > m.max)
			m.cur = m.max;
	}

	bool castSpell(Mana& m, const SpellDef& s, Vector3 eye, Vector3 aim, std::vector<Enemy>& enemies, std::vector<Projectile>& shots, float& playerHealth, float maxHealth, const EnemyTuning& t)
	{
		if (m.cur < s.manaCost)
			return false;
		m.cur -= s.manaCost;

		if (s.kind == Cast_Heal)
		{
			playerHealth += s.power;
			if (playerHealth > maxHealth)
				playerHealth = maxHealth;
			return true;
		}

		if (s.kind == Cast_Projectile)
		{
			Projectile p;
			p.position = {eye.x + aim.x * 0.4f, eye.y + aim.y * 0.4f, eye.z + aim.z * 0.4f};
			p.velocity = {aim.x * s.projectileSpeed, aim.y * s.projectileSpeed, aim.z * s.projectileSpeed};
			p.dir = aim;
			p.damage = s.power;
			p.color = schoolColor(s.school);
			p.effect = schoolEffect(s.school);
			shots.push_back(p);
			return true;
		}

		// Push / Melee: act on enemies in a forward cone (horizontal aim).
		const float hlen = std::sqrt(aim.x * aim.x + aim.z * aim.z);
		const float fx = hlen > 0.0001f ? aim.x / hlen : 0.0f;
		const float fz = hlen > 0.0001f ? aim.z / hlen : -1.0f;
		for (Enemy& e : enemies)
		{
			if (!e.active || e.state == EnemyState::Dead)
				continue;
			const float dx = e.position.x - eye.x;
			const float dz = e.position.z - eye.z;
			const float d = std::sqrt(dx * dx + dz * dz);
			if (d > s.range)
				continue;
			if (d > 0.0001f && (fx * dx + fz * dz) / d < 0.4f) // front cone
				continue;

			const float nx = d > 0.0001f ? dx / d : fx;
			const float nz = d > 0.0001f ? dz / d : fz;
			if (s.kind == Cast_Melee)
			{
				e.health -= s.power;
				e.velocity.x += nx * s.power * 0.3f; // Earth throws them back
				e.velocity.z += nz * s.power * 0.3f;
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
			}
			else // Cast_Push
			{
				e.velocity.x += nx * s.power;
				e.velocity.z += nz * s.power;
				e.state = EnemyState::Stagger;
				e.stateTimer = t.staggerTime;
			}
		}
		return true;
	}
} // namespace adventure
