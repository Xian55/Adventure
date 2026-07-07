#include <doctest/doctest.h>
#include "combat/Melee.h"

using namespace adventure;

namespace
{
	WeaponDef def()
	{
		WeaponDef d;
		d.windup = 0.10f;
		d.active = 0.05f;
		d.recovery = 0.20f;
		return d;
	}

	void advance(MeleeState& s, const WeaponDef& d, float total)
	{
		const float dt = 1.0f / 60.0f;
		for (float t = 0.0f; t < total; t += dt)
			updateMelee(s, d, dt);
	}
} // namespace

TEST_CASE("swing cycles Idle -> Windup -> Active -> Recovery -> Idle")
{
	WeaponDef d = def();
	MeleeState s;
	CHECK(s.phase == MeleePhase::Idle);

	requestSwing(s);
	updateMelee(s, d, 0.0f); // Idle consumes the buffered swing
	CHECK(s.phase == MeleePhase::Windup);

	advance(s, d, d.windup + 0.001f);
	CHECK(s.phase == MeleePhase::Active);
	CHECK(hitboxActive(s)); // hitbox live only during Active

	advance(s, d, d.active + 0.001f);
	CHECK(s.phase == MeleePhase::Recovery);
	CHECK_FALSE(hitboxActive(s));

	advance(s, d, d.recovery + 0.02f);
	CHECK(s.phase == MeleePhase::Idle);
}

TEST_CASE("a buffered input chains into a combo during Recovery")
{
	WeaponDef d = def();
	MeleeState s;
	requestSwing(s);
	updateMelee(s, d, 0.0f);
	advance(s, d, d.windup + 0.001f);
	advance(s, d, d.active + 0.001f);
	REQUIRE(s.phase == MeleePhase::Recovery);

	requestSwing(s);
	updateMelee(s, d, 1.0f / 60.0f);
	CHECK(s.phase == MeleePhase::Windup);
	CHECK(s.comboStep == 1);
}

TEST_CASE("hitThisSwing is cleared at each swing start")
{
	WeaponDef d = def();
	MeleeState s;
	requestSwing(s);
	updateMelee(s, d, 0.0f);
	s.hitThisSwing = true; // pretend a hit landed this swing
	advance(s, d, d.windup + 0.001f);
	advance(s, d, d.active + 0.001f); // Recovery
	requestSwing(s);
	updateMelee(s, d, 1.0f / 60.0f); // new swing
	CHECK_FALSE(s.hitThisSwing);
}

TEST_CASE("phaseProgress stays in 0..1 and handles zero-duration phases")
{
	WeaponDef d;
	d.windup = 0.0f;
	d.active = 0.0f;
	d.recovery = 0.0f;
	MeleeState s;
	requestSwing(s);
	updateMelee(s, d, 0.0f); // Windup, but windup==0
	CHECK(phaseProgress(s, d) == doctest::Approx(1.0));
	CHECK(phaseProgress(s, def()) >= 0.0f);

	updateMelee(s, d, 1.0f / 60.0f); // zero-duration windup advances immediately
	CHECK(s.phase == MeleePhase::Active);
}
