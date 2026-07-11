#pragma once
#include "world/MapTypes.h"

#include <vector>

namespace adventure::world
{
	// Map-space brushes -> engine-space render meshes (grouped by texture) + convex collision brushes.
	// Winding-robust: face-plane normals are oriented outward from an interior point, so authored point
	// order does not matter. `trigger_*` entities are skipped (they are sensors, not solid geometry).
	// See src/world/CLAUDE.md.
	WorldGeometry buildWorld(const MapData& map);

	// Damage sensors from trigger_hurt entities (engine-space AABBs). `dmg` key = damage per second.
	std::vector<Hazard> buildHazards(const MapData& map);

	bool hazardContains(const Hazard& h, Vector3 p);
	float hazardDamageAt(const std::vector<Hazard>& hazards, Vector3 p); // summed dmg/sec at p, 0 if none
} // namespace adventure::world
