#include "combat/Rage.h"

namespace adventure
{
	void addRage(RageState& s, const RageTuning& t, int hits, int kills)
	{
		if (hits <= 0 && kills <= 0)
			return;
		s.sinceHit = 0.0f; // fighting -> hold off decay
		if (s.berserk)
			return; // already maxed; hits don't extend it
		s.rage += hits * t.gainPerHit + kills * t.gainPerKill;
		if (s.rage >= t.max)
		{
			s.rage = t.max;
			s.berserk = true;
			s.berserkTimer = t.berserkDuration;
		}
	}

	void updateRage(RageState& s, const RageTuning& t, float dt)
	{
		if (s.berserk)
		{
			s.berserkTimer -= dt;
			// Drain the meter across the berserk window so the HUD reads down to empty as it ends.
			const float frac = t.berserkDuration > 0.0f ? s.berserkTimer / t.berserkDuration : 0.0f;
			s.rage = t.max * (frac > 0.0f ? frac : 0.0f);
			if (s.berserkTimer <= 0.0f)
			{
				s.berserk = false;
				s.berserkTimer = 0.0f;
				s.rage = 0.0f;
				s.sinceHit = 0.0f;
			}
			return;
		}

		s.sinceHit += dt;
		if (s.sinceHit >= t.decayDelay && s.rage > 0.0f)
		{
			s.rage -= t.decayPerSec * dt;
			if (s.rage < 0.0f)
				s.rage = 0.0f;
		}
	}

	float rageFraction(const RageState& s, const RageTuning& t)
	{
		if (t.max <= 0.0f)
			return 0.0f;
		const float f = s.rage / t.max;
		return f < 0.0f ? 0.0f : (f > 1.0f ? 1.0f : f);
	}

	float rageDamageMul(const RageState& s, const RageTuning& t)
	{
		return s.berserk ? t.damageMul : 1.0f;
	}

	float rageSpeedMul(const RageState& s, const RageTuning& t)
	{
		return s.berserk ? t.speedMul : 1.0f;
	}
} // namespace adventure
