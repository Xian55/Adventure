#pragma once
#include "raylib.h"

namespace adventure
{
	// First-person player state, engine space. `position` is the AABB center; feet = position.y - height/2.
	struct Player
	{
		Vector3 position{0, 0, 0};
		Vector3 velocity{0, 0, 0};
		float yaw = 0.0f;   // radians; 0 looks toward -Z
		float pitch = 0.0f; // radians; clamped
		float health = 100.0f;
		float maxHealth = 100.0f;
		bool onGround = false;
		bool crouched = false;
	};
} // namespace adventure
