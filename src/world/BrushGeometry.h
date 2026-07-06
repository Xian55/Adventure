#pragma once
#include "world/MapTypes.h"

namespace adventure::world
{
	// Map-space brushes -> engine-space render meshes (grouped by texture) + convex collision brushes.
	// Winding-robust: face-plane normals are oriented outward from an interior point, so authored point
	// order does not matter. See src/world/CLAUDE.md.
	WorldGeometry buildWorld(const MapData& map);
} // namespace adventure::world
