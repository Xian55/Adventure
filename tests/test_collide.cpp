#include <doctest/doctest.h>
#include "items/Collide.h"

using namespace adventure;

namespace
{
	SolidBox box(float cx, float cy, float cz, float h = 0.5f)
	{
		return SolidBox{{cx, cy, cz}, {h, h, h}};
	}
} // namespace

TEST_CASE("an actor falling onto a box lands on top and is grounded")
{
	std::vector<SolidBox> solids{box(0, 0, 0)}; // top at y = 0.5
	Vector3 pos = {0, 1.35f, 0};                // he.y 0.9 -> feet at 0.45, just inside the top
	Vector3 vel = {0, -5.0f, 0};
	bool onGround = false;
	collideActorBoxes(pos, vel, onGround, Vector3{0.3f, 0.9f, 0.3f}, solids);
	CHECK(pos.y == doctest::Approx(1.4)); // feet snapped to the box top (0.5 + 0.9)
	CHECK(vel.y == doctest::Approx(0.0)); // downward speed killed
	CHECK(onGround);
}

TEST_CASE("an actor overlapping the side is pushed out horizontally")
{
	std::vector<SolidBox> solids{box(0, 0.5f, 0)};
	Vector3 pos = {0.6f, 0.5f, 0.0f}; // deep vertical overlap, shallow x overlap -> push x
	Vector3 vel = {0, 0, 0};
	bool onGround = false;
	collideActorBoxes(pos, vel, onGround, Vector3{0.3f, 0.4f, 0.3f}, solids);
	CHECK(pos.x == doctest::Approx(0.8)); // pushed to box edge (0.5) + he.x (0.3)
	CHECK_FALSE(onGround);
}

TEST_CASE("an actor jumping into the underside bonks its head")
{
	std::vector<SolidBox> solids{box(0, 2.0f, 0)}; // bottom at y = 1.5
	Vector3 pos = {0, 0.65f, 0};                   // he.y 0.9 -> head at 1.55, just inside the bottom
	Vector3 vel = {0, 4.0f, 0};
	bool onGround = false;
	collideActorBoxes(pos, vel, onGround, Vector3{0.3f, 0.9f, 0.3f}, solids);
	CHECK(pos.y == doctest::Approx(0.6)); // head pushed below the box (1.5 - 0.9)
	CHECK(vel.y == doctest::Approx(0.0)); // upward speed killed
	CHECK_FALSE(onGround);
}

TEST_CASE("a clear actor is untouched")
{
	std::vector<SolidBox> solids{box(0, 0.5f, 0)};
	Vector3 pos = {3, 0.9f, 0};
	Vector3 vel = {1, -1, 0};
	bool onGround = false;
	collideActorBoxes(pos, vel, onGround, Vector3{0.3f, 0.9f, 0.3f}, solids);
	CHECK(pos.x == doctest::Approx(3.0));
	CHECK(vel.y == doctest::Approx(-1.0));
	CHECK_FALSE(onGround);
}
