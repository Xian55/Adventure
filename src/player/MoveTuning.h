#pragma once

namespace adventure
{
	// Movement feel constants. Loaded from Lua (scripts/tuning.lua) so feel is hot-reloadable; the values
	// here are fallbacks. Dimensions (radius/height/eye) are structural and stay in C++.
	struct MoveTuning
	{
		float moveSpeed = 4.5f;   // walk
		float sprintSpeed = 8.0f; // hold sprint
		float accel = 12.0f;
		float airAccel = 1.0f;
		float friction = 9.0f;
		float stopSpeed = 3.0f;
		float gravity = 25.0f;
		float jumpSpeed = 7.5f;

		float radius = 0.35f; // XZ half-extent
		float height = 1.7f;  // standing full height
		float crouchHeight = 1.0f;
		float eyeHeight = 1.5f; // eye above feet (standing)
	};
} // namespace adventure
