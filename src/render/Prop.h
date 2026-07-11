#pragma once
#include "raylib.h"
#include "combat/Destructible.h"

#include <vector>

// Placeholder 3D geometry for destructible props + loot pickups (barrels/kegs = cylinders, crates = cubes),
// with a debris puff on break. Procedural until real models/sprites land.
namespace adventure
{
	void drawProps(const std::vector<Destructible>& props, float time);
	void drawPickups(const std::vector<Pickup>& pickups, float time);
} // namespace adventure
