#include "combat/Spell.h"

#include <cmath>

namespace adventure
{
	namespace
	{
		SpellDef kSpells[SPELL_COUNT] = {
		    {"Telekinesis", 25.0f, 18.0f, 6.0f, 0.4f},
		};
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

	void updateMana(Mana& m, float dt)
	{
		m.cur += m.regen * dt;
		if (m.cur > m.max)
			m.cur = m.max;
	}

	bool castPush(Mana& m, const SpellDef& s, Vector3 origin, float yaw, std::vector<Enemy>& enemies, const EnemyTuning& t)
	{
		if (m.cur < s.manaCost)
			return false;
		m.cur -= s.manaCost;

		const float fx = std::sin(yaw);
		const float fz = -std::cos(yaw);
		for (Enemy& e : enemies)
		{
			if (!e.active || e.state == EnemyState::Dead)
				continue;
			const float tx = e.position.x - origin.x;
			const float tz = e.position.z - origin.z;
			const float d = std::sqrt(tx * tx + tz * tz);
			if (d > s.range)
				continue;
			if (d > 0.0001f && (fx * tx + fz * tz) / d < s.coneCos) // outside the cone
				continue;

			// shove along the direction to the enemy (so they fly away from the caster)
			const float nx = d > 0.0001f ? tx / d : fx;
			const float nz = d > 0.0001f ? tz / d : fz;
			e.velocity.x += nx * s.power;
			e.velocity.z += nz * s.power;
			e.state = EnemyState::Stagger;
			e.stateTimer = t.staggerTime;
		}
		return true;
	}
} // namespace adventure
