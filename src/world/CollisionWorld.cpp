#include "world/CollisionWorld.h"

#include <cmath>

namespace adventure::world
{
	void CollisionWorld::build(const WorldGeometry& geo)
	{
		m_brushes = geo.collision;
	}

	bool CollisionWorld::overlaps(Vector3 c, Vector3 h) const
	{
		const Vector3 boxMin = {c.x - h.x, c.y - h.y, c.z - h.z};
		const Vector3 boxMax = {c.x + h.x, c.y + h.y, c.z + h.z};

		for (const CollisionBrush& b : m_brushes)
		{
			// Broadphase: skip brushes whose AABB doesn't overlap the query box.
			if (boxMax.x < b.min.x || boxMin.x > b.max.x || boxMax.y < b.min.y || boxMin.y > b.max.y ||
			    boxMax.z < b.min.z || boxMin.z > b.max.z)
				continue;

			// Narrowphase: the box overlaps the convex brush iff its center is inside every face plane
			// pushed outward by the box's support along that plane's normal (Minkowski expansion).
			bool inside = true;
			for (Vector4 pl : b.planes)
			{
				float support = std::fabs(pl.x) * h.x + std::fabs(pl.y) * h.y + std::fabs(pl.z) * h.z;
				float dist = pl.x * c.x + pl.y * c.y + pl.z * c.z + pl.w;
				if (dist > support)
				{
					inside = false;
					break;
				}
			}
			if (inside)
				return true;
		}
		return false;
	}
} // namespace adventure::world
