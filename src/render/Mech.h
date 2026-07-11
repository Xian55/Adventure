#pragma once
#include "mech/Mechanisms.h"

#include <vector>

// Placeholder 3D for doors / levers / pressure plates. Procedural until real models. Presentation only.
namespace adventure
{
	void drawDoors(const std::vector<Door>& doors);
	void drawLevers(const std::vector<Lever>& levers);
	void drawPlates(const std::vector<Plate>& plates);
} // namespace adventure
