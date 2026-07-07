#include <doctest/doctest.h>
#include "player/JumpMeter.h"

using namespace adventure;

TEST_CASE("records a jump's distance, apex height, and airtime")
{
	JumpMeter m;
	Player p;
	p.position = {0, 1, 0};
	p.onGround = true;
	m.update(p, 0.1f);
	CHECK_FALSE(m.airborne());

	p.onGround = false; // takeoff from (0,1,0)
	m.update(p, 0.1f);
	CHECK(m.airborne());
	p.position = {1.5f, 2.0f, 0}; // apex: +1.0 above launch
	m.update(p, 0.1f);
	p.position = {3.0f, 1.2f, 0};
	m.update(p, 0.1f);
	p.onGround = true; // land at (4,1,0)
	p.position = {4.0f, 1.0f, 0};
	m.update(p, 0.1f);

	CHECK_FALSE(m.airborne());
	CHECK(m.last().distance == doctest::Approx(4.0));
	CHECK(m.last().height == doctest::Approx(1.0));
	CHECK(m.last().airtime == doctest::Approx(0.3)); // 3 airborne updates * 0.1s
	CHECK(m.maxDistance() == doctest::Approx(4.0));
	CHECK(m.maxHeight() == doctest::Approx(1.0));
}

TEST_CASE("last reflects the latest jump; maxima persist")
{
	JumpMeter m;
	Player p;

	// jump 1: distance 4, height 1
	p.onGround = true;
	p.position = {0, 1, 0};
	m.update(p, 0.1f);
	p.onGround = false;
	m.update(p, 0.1f);
	p.position = {2, 2, 0};
	m.update(p, 0.1f);
	p.onGround = true;
	p.position = {4, 1, 0};
	m.update(p, 0.1f);

	// jump 2: shorter distance 2, lower height 0.5
	p.onGround = false;
	m.update(p, 0.1f);
	p.position = {5, 1.5f, 0};
	m.update(p, 0.1f);
	p.onGround = true;
	p.position = {6, 1, 0};
	m.update(p, 0.1f);

	CHECK(m.last().distance == doctest::Approx(2.0)); // latest jump
	CHECK(m.last().height == doctest::Approx(0.5));
	CHECK(m.maxDistance() == doctest::Approx(4.0)); // maxima persist
	CHECK(m.maxHeight() == doctest::Approx(1.0));
}
