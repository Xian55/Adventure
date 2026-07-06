#pragma once

namespace adventure
{
	// Movement feel constants. Loaded from Lua (scripts/tuning.lua) so feel is hot-reloadable; the values
	// here are fallbacks. Dimensions (radius/height/eye) are structural and stay in C++.
	struct MoveTuning
	{
		float moveSpeed = 8.0f;
		float accel = 10.0f;
		float airAccel = 1.2f;
		float friction = 6.0f;
		float stopSpeed = 2.0f;
		float gravity = 22.0f;
		float jumpSpeed = 8.0f;

		float radius = 0.35f; // XZ half-extent
		float height = 1.7f;  // standing full height
		float crouchHeight = 1.0f;
		float eyeHeight = 1.5f; // eye above feet (standing)
	};
} // namespace adventure
