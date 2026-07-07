#include "combat/Melee.h"

namespace adventure
{
	void beginCharge(MeleeState& s)
	{
		if (s.phase != MeleePhase::Idle)
			return;
		s.phase = MeleePhase::Charge;
		s.timer = 0.0f;
		s.chargeTime = 0.0f;
		s.dir = SwingDir::Neutral;
		s.dirLocked = false;
		s.hitThisSwing = false;
	}

	// Snapshot the first direction chosen during the charge; ignore later key changes (and Neutral, so a
	// key pressed after the button still counts). The picked direction sticks until release.
	void setSwingDir(MeleeState& s, SwingDir d)
	{
		if (s.phase != MeleePhase::Charge || s.dirLocked || d == SwingDir::Neutral)
			return;
		s.dir = d;
		s.dirLocked = true;
	}

	void releaseSwing(MeleeState& s)
	{
		if (s.phase != MeleePhase::Charge)
			return;
		// Neutral (no direction held) alternates Left/Right so hold-release-repeat reads as a flurry.
		if (s.dir == SwingDir::Neutral)
		{
			s.resolved = s.neutralLeft ? SwingDir::Left : SwingDir::Right;
			s.neutralLeft = !s.neutralLeft;
		}
		else
		{
			s.resolved = s.dir;
		}
		s.phase = MeleePhase::Active;
		s.timer = 0.0f;
	}

	void updateMelee(MeleeState& s, const WeaponDef& def, float dt)
	{
		switch (s.phase)
		{
		case MeleePhase::Idle:
			break;
		case MeleePhase::Charge:
			s.chargeTime += dt; // player holds until release
			break;
		case MeleePhase::Active:
			s.timer += dt;
			if (s.timer >= def.active)
			{
				s.phase = MeleePhase::Recovery;
				s.timer = 0.0f;
			}
			break;
		case MeleePhase::Recovery:
			s.timer += dt;
			if (s.timer >= def.recovery)
			{
				s.phase = MeleePhase::Idle;
				s.timer = 0.0f;
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
		if (s.phase == MeleePhase::Active)
			d = def.active;
		else if (s.phase == MeleePhase::Recovery)
			d = def.recovery;
		else
			return 0.0f;
		if (d <= 0.0f)
			return 1.0f;
		float p = s.timer / d;
		return p < 0.0f ? 0.0f : (p > 1.0f ? 1.0f : p);
	}

	float chargeFraction(const MeleeState& s, const WeaponDef& def)
	{
		if (def.chargeMax <= 0.0f)
			return 1.0f;
		float f = s.chargeTime / def.chargeMax;
		return f < 0.0f ? 0.0f : (f > 1.0f ? 1.0f : f);
	}
} // namespace adventure
