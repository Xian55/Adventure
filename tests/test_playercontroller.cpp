#include <doctest/doctest.h>
#include "map_helpers.h"
#include "player/PlayerController.h"
#include "world/BrushGeometry.h"
#include "world/MapParser.h"

using namespace adventure;

namespace
{
	world::CollisionWorld worldFrom(const std::string& mapText)
	{
		world::MapParseResult r = world::parseMap(mapText);
		REQUIRE(r.ok);
		world::CollisionWorld w;
		w.build(world::buildWorld(r.data));
		return w;
	}

	void step(Player& p, const MoveInput& in, const world::CollisionWorld& w, const MoveTuning& t, int n)
	{
		for (int i = 0; i < n; ++i)
			updatePlayer(p, in, w, t, 1.0f / 60.0f);
	}
} // namespace

TEST_CASE("player falls and lands on the floor")
{
	// Floor: map z in [-64,0] -> engine y in [-2,0], top at y=0.
	world::CollisionWorld w = worldFrom(boxMap(-256, 256, -256, 256, -64, 0));
	MoveTuning t;
	Player p;
	p.position = {0.0f, 3.0f, 0.0f};

	step(p, MoveInput{}, w, t, 300);

	CHECK(p.onGround);
	// Rests with the AABB bottom on the floor top: center ~ height/2 above y=0.
	CHECK(p.position.y == doctest::Approx(t.height * 0.5f).epsilon(0.06));
	CHECK(p.velocity.y == doctest::Approx(0.0).epsilon(0.01));
}

TEST_CASE("a wall blocks horizontal movement")
{
	// Solid wall occupying engine x >= 0 (map x in [0,256]).
	world::CollisionWorld w = worldFrom(boxMap(0, 256, -256, 256, -256, 256));
	MoveTuning t;
	t.gravity = 0.0f; // isolate horizontal motion
	t.friction = 0.0f;

	Player p;
	p.position = {-2.0f, 0.5f, 0.0f};
	p.onGround = true; // so ground accel applies

	MoveInput in;
	in.right = 1.0f; // yaw 0: right is +X, toward the wall

	step(p, in, w, t, 240);

	// Blocked before entering the solid: center stops around -radius, never crossing x=0.
	CHECK(p.position.x < 0.0f);
	CHECK(p.position.x <= -t.radius + 0.05f);
	CHECK(p.position.x > -2.0f); // but it did move toward the wall
}

TEST_CASE("crouch then stand keeps the player mobile (no stuck-after-crouch)")
{
	world::CollisionWorld w = worldFrom(boxMap(-256, 256, -256, 256, -64, 0)); // floor top at y=0
	MoveTuning t;
	Player p;
	p.position = {0.0f, 3.0f, 0.0f};
	step(p, MoveInput{}, w, t, 240); // land
	REQUIRE(p.onGround);

	MoveInput crouch;
	crouch.crouch = true;
	step(p, crouch, w, t, 30);
	CHECK(p.crouched);
	CHECK(p.onGround);

	// can move while crouched
	float x0 = p.position.x;
	MoveInput crouchMove;
	crouchMove.crouch = true;
	crouchMove.right = 1.0f;
	step(p, crouchMove, w, t, 90);
	CHECK(p.position.x > x0 + 0.05f);

	// stand up (open room -> headroom) and stay mobile (the bug: stuck after crouch)
	MoveInput stand;
	step(p, stand, w, t, 30);
	CHECK_FALSE(p.crouched);
	float x1 = p.position.x;
	MoveInput move;
	move.right = 1.0f;
	step(p, move, w, t, 90);
	CHECK(p.position.x > x1 + 0.05f);
}

TEST_CASE("player can move away from a wall unobstructed")
{
	world::CollisionWorld w = worldFrom(boxMap(0, 256, -256, 256, -256, 256));
	MoveTuning t;
	t.gravity = 0.0f;

	Player p;
	p.position = {-1.0f, 0.5f, 0.0f};
	p.onGround = true;

	MoveInput in;
	in.right = -1.0f; // away from the wall (toward -X)

	step(p, in, w, t, 120);
	CHECK(p.position.x < -1.0f);
}
