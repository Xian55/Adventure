#pragma once
#include "raylib.h"

#include <vector>

// Actor-vs-solid-box collision with vertical support, so props/chests can be walked into AND stood on.
// Pure logic (no raylib window) -> headless-testable.
namespace adventure
{
	struct SolidBox
	{
		Vector3 center{0, 0, 0};
		Vector3 half{0.5f, 0.5f, 0.5f};
	};

	// Resolve an actor AABB (center `pos`, half-extents `he`) against each solid box by minimum penetration:
	// a side hit pushes horizontally; landing on top snaps up, zeroes downward velocity, and sets `onGround`;
	// a head bump pushes down and zeroes upward velocity. `pos`/`vel`/`onGround` are updated in place. Pure.
	void collideActorBoxes(Vector3& pos, Vector3& vel, bool& onGround, Vector3 he, const std::vector<SolidBox>& solids);
} // namespace adventure
