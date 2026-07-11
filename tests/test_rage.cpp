#include <doctest/doctest.h>
#include "combat/Rage.h"

using namespace adventure;

namespace
{
	void advance(RageState& s, const RageTuning& t, float total)
	{
		const float dt = 1.0f / 60.0f;
		for (float e = 0.0f; e < total; e += dt)
			updateRage(s, t, dt);
	}
} // namespace

TEST_CASE("landed hits build rage; kills give more")
{
	RageTuning t;
	RageState s;
	addRage(s, t, 1, 0);
	CHECK(s.rage == doctest::Approx(t.gainPerHit));
	addRage(s, t, 1, 1); // a hit that also kills
	CHECK(s.rage == doctest::Approx(t.gainPerHit + t.gainPerHit + t.gainPerKill));
	CHECK_FALSE(s.berserk);
}

TEST_CASE("nothing connecting does not build rage")
{
	RageTuning t;
	RageState s;
	addRage(s, t, 0, 0);
	CHECK(s.rage == doctest::Approx(0.0));
}

TEST_CASE("filling the meter triggers berserk with its multipliers")
{
	RageTuning t;
	RageState s;
	CHECK(rageDamageMul(s, t) == doctest::Approx(1.0));
	CHECK(rageSpeedMul(s, t) == doctest::Approx(1.0));

	addRage(s, t, 100, 0); // way over max
	CHECK(s.berserk);
	CHECK(s.rage == doctest::Approx(t.max)); // clamped
	CHECK(rageDamageMul(s, t) == doctest::Approx(t.damageMul));
	CHECK(rageSpeedMul(s, t) == doctest::Approx(t.speedMul));
}

TEST_CASE("berserk drains over its duration then resets to zero")
{
	RageTuning t;
	RageState s;
	addRage(s, t, 100, 0);
	CHECK(s.berserk);
	advance(s, t, t.berserkDuration * 0.5f);
	CHECK(s.berserk);
	CHECK(rageFraction(s, t) == doctest::Approx(0.5).epsilon(0.05)); // meter half-drained mid-window

	advance(s, t, t.berserkDuration * 0.5f + 0.1f);
	CHECK_FALSE(s.berserk);
	CHECK(s.rage == doctest::Approx(0.0));
	CHECK(rageDamageMul(s, t) == doctest::Approx(1.0));
}

TEST_CASE("hits during berserk do not extend or restack it")
{
	RageTuning t;
	RageState s;
	addRage(s, t, 100, 0);
	const float before = s.berserkTimer;
	addRage(s, t, 5, 2);                              // swinging while berserk
	CHECK(s.berserkTimer == doctest::Approx(before)); // unchanged
	CHECK(s.rage == doctest::Approx(t.max));          // not pushed past max
}

TEST_CASE("rage only decays after the grace delay, then bleeds to zero")
{
	RageTuning t;
	RageState s;
	addRage(s, t, 2, 0); // some rage, below max
	const float built = s.rage;

	advance(s, t, t.decayDelay - 0.2f); // still inside the grace window
	CHECK(s.rage == doctest::Approx(built));

	advance(s, t, 0.5f); // past the delay -> decaying
	CHECK(s.rage < built);

	advance(s, t, built / t.decayPerSec + 1.0f);
	CHECK(s.rage == doctest::Approx(0.0));
}
