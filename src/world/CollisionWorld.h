#pragma once
#include "raylib.h"
#include "world/MapTypes.h"

#include <vector>

namespace adventure::world
{
	// Convex-brush collision queries. Pure math (no raylib window) -> headless-testable. Broadphase is a
	// linear AABB filter for now (see the partitioning plan in docs/design/ARCHITECTURE.md).
	class CollisionWorld
	{
	public:
		void build(const WorldGeometry& geo);
		int brushCount() const { return (int)m_brushes.size(); }

		// True if the axis-aligned box (center +/- halfExtents) overlaps any solid brush.
		bool overlaps(Vector3 center, Vector3 halfExtents) const;

	private:
		std::vector<CollisionBrush> m_brushes;
	};
} // namespace adventure::world
