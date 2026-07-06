#pragma once
#include "player/MoveTuning.h"
#include "player/Player.h"
#include "world/CollisionWorld.h"

namespace adventure
{
	struct MoveInput
	{
		float forward = 0.0f; // -1..1 (W/S)
		float right = 0.0f;   // -1..1 (D/A)
		bool jump = false;
		bool crouch = false;
	};

	// One fixed-step update: Quake-style accelerate/friction + axis-separated swept-AABB collision.
	void updatePlayer(Player& p, const MoveInput& in, const world::CollisionWorld& world, const MoveTuning& t, float dt);
} // namespace adventure
