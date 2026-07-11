#include "items/Collide.h"

#include <cmath>

namespace adventure
{
	void collideActorBoxes(Vector3& pos, Vector3& vel, bool& onGround, Vector3 he, const std::vector<SolidBox>& solids)
	{
		for (const SolidBox& s : solids)
		{
			const float ax0 = pos.x - he.x, ax1 = pos.x + he.x;
			const float ay0 = pos.y - he.y, ay1 = pos.y + he.y;
			const float az0 = pos.z - he.z, az1 = pos.z + he.z;
			const float bx0 = s.center.x - s.half.x, bx1 = s.center.x + s.half.x;
			const float by0 = s.center.y - s.half.y, by1 = s.center.y + s.half.y;
			const float bz0 = s.center.z - s.half.z, bz1 = s.center.z + s.half.z;

			if (ax1 <= bx0 || ax0 >= bx1 || ay1 <= by0 || ay0 >= by1 || az1 <= bz0 || az0 >= bz1)
				continue; // no overlap

			const float px = fminf(ax1 - bx0, bx1 - ax0);
			const float py = fminf(ay1 - by0, by1 - ay0);
			const float pz = fminf(az1 - bz0, bz1 - az0);

			if (px <= py && px <= pz) // resolve along X (side)
			{
				pos.x += (pos.x < s.center.x) ? -px : px;
			}
			else if (pz <= py) // resolve along Z (side)
			{
				pos.z += (pos.z < s.center.z) ? -pz : pz;
			}
			else if (pos.y > s.center.y) // land on top
			{
				pos.y += py;
				if (vel.y < 0.0f)
					vel.y = 0.0f;
				onGround = true;
			}
			else // bonk head
			{
				pos.y -= py;
				if (vel.y > 0.0f)
					vel.y = 0.0f;
			}
		}
	}
} // namespace adventure
