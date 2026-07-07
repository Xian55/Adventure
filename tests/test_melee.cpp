#include <doctest/doctest.h>
#include "combat/Melee.h"

using namespace adventure;

namespace
{
	WeaponDef def()
	{
		WeaponDef d;
		d.active = 0.05f;
		d.recovery = 0.20f;
		d.chargeMax = 0.5f;
		return d;
	}

	void advance(MeleeState& s, const WeaponDef& d, float total)
	{
		const float dt = 1.0f / 60.0f;
		for (float t = 0.0f; t < total; t += dt)
			updateMelee(s, d, dt);
	}

	void fullSwing(MeleeState& s, const WeaponDef& d) // release -> back to Idle
	{
		releaseSwing(s);
		advance(s, d, d.active + d.recovery + 0.05f);
	}
} // namespace

TEST_CASE("charge then release: Idle -> Charge -> Active -> Recovery -> Idle")
{
	WeaponDef d = def();
	MeleeState s;
	CHECK(s.phase == MeleePhase::Idle);

	beginCharge(s);
	CHECK(s.phase == MeleePhase::Charge);
	advance(s, d, 0.2f);
	CHECK(s.phase == MeleePhase::Charge); // stays charging until release
	CHECK(s.chargeTime == doctest::Approx(0.2).epsilon(0.03));

	releaseSwing(s);
	CHECK(s.phase == MeleePhase::Active);
	CHECK(hitboxActive(s));

	advance(s, d, d.active + 0.001f);
	CHECK(s.phase == MeleePhase::Recovery);
	CHECK_FALSE(hitboxActive(s));

	advance(s, d, d.recovery + 0.02f);
	CHECK(s.phase == MeleePhase::Idle);
}

TEST_CASE("held direction is used; Neutral alternates Left/Right")
{
	WeaponDef d = def();
	MeleeState s;

	beginCharge(s);
	setSwingDir(s, SwingDir::Overhead);
	releaseSwing(s);
	CHECK(s.resolved == SwingDir::Overhead);
	advance(s, d, d.active + d.recovery + 0.05f);

	beginCharge(s); // no direction -> Neutral -> Left
	fullSwing(s, d);
	CHECK(s.resolved == SwingDir::Left);

	beginCharge(s); // Neutral again -> Right
	fullSwing(s, d);
	CHECK(s.resolved == SwingDir::Right);
}

TEST_CASE("swing direction snapshots at first pick and ignores later key changes")
{
	WeaponDef d = def();
	MeleeState s;

	// First non-neutral key wins; switching keys mid-charge does nothing.
	beginCharge(s);
	setSwingDir(s, SwingDir::Left);
	setSwingDir(s, SwingDir::Right);    // ignored — already locked
	setSwingDir(s, SwingDir::Overhead); // ignored
	releaseSwing(s);
	CHECK(s.resolved == SwingDir::Left);
	advance(s, d, d.active + d.recovery + 0.05f);

	// A key pressed a little after the button (Neutral first) still registers and then locks.
	beginCharge(s);
	setSwingDir(s, SwingDir::Neutral); // no key yet -> does not lock
	setSwingDir(s, SwingDir::Forward); // pressed during the hold -> locks
	setSwingDir(s, SwingDir::Left);    // ignored
	releaseSwing(s);
	CHECK(s.resolved == SwingDir::Forward);
}

TEST_CASE("chargeFraction scales 0..1 and clamps at chargeMax")
{
	WeaponDef d = def(); // chargeMax 0.5
	MeleeState s;
	beginCharge(s);
	advance(s, d, 0.25f);
	CHECK(chargeFraction(s, d) == doctest::Approx(0.5).epsilon(0.06));
	advance(s, d, 1.0f);
	CHECK(chargeFraction(s, d) == doctest::Approx(1.0));
}

TEST_CASE("transitions are ignored out of context")
{
	MeleeState s;
	setSwingDir(s, SwingDir::Left); // no-op in Idle
	releaseSwing(s);                // no-op in Idle
	CHECK(s.phase == MeleePhase::Idle);

	beginCharge(s);
	releaseSwing(s); // Active
	beginCharge(s);  // ignored (not Idle)
	CHECK(s.phase == MeleePhase::Active);
}

TEST_CASE("hitThisSwing is cleared at each charge start")
{
	WeaponDef d = def();
	MeleeState s;
	beginCharge(s);
	fullSwing(s, d);
	s.hitThisSwing = true;
	beginCharge(s);
	CHECK_FALSE(s.hitThisSwing);
}
