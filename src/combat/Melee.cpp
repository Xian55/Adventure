#include "combat/Melee.h"

namespace adventure
{
	void requestSwing(MeleeState& s)
	{
		s.wantSwing = true;
	}

	namespace
	{
		void beginWindup(MeleeState& s)
		{
			s.phase = MeleePhase::Windup;
			s.timer = 0.0f;
			s.wantSwing = false;
			s.hitThisSwing = false;
		}
	} // namespace

	void updateMelee(MeleeState& s, const WeaponDef& def, float dt)
	{
		if (s.phase != MeleePhase::Idle)
			s.timer += dt;

		switch (s.phase)
		{
		case MeleePhase::Idle:
			if (s.wantSwing)
			{
				s.comboStep = 0;
				beginWindup(s);
			}
			break;
		case MeleePhase::Windup:
			if (s.timer >= def.windup)
			{
				s.phase = MeleePhase::Active;
				s.timer = 0.0f;
			}
			break;
		case MeleePhase::Active:
			if (s.timer >= def.active)
			{
				s.phase = MeleePhase::Recovery;
				s.timer = 0.0f;
			}
			break;
		case MeleePhase::Recovery:
			if (s.wantSwing) // chain into the next swing (combo)
			{
				s.comboStep = (s.comboStep + 1) % 3;
				beginWindup(s);
			}
			else if (s.timer >= def.recovery)
			{
				s.phase = MeleePhase::Idle;
				s.timer = 0.0f;
				s.comboStep = 0;
			}
			break;
		}
	}

	bool hitboxActive(const MeleeState& s)
	{
		return s.phase == MeleePhase::Active;
	}

	float phaseProgress(const MeleeState& s, const WeaponDef& def)
	{
		float d = 0.0f;
		switch (s.phase)
		{
		case MeleePhase::Windup:
			d = def.windup;
			break;
		case MeleePhase::Active:
			d = def.active;
			break;
		case MeleePhase::Recovery:
			d = def.recovery;
			break;
		default:
			return 0.0f;
		}
		if (d <= 0.0f)
			return 1.0f;
		float p = s.timer / d;
		return p < 0.0f ? 0.0f : (p > 1.0f ? 1.0f : p);
	}
} // namespace adventure
