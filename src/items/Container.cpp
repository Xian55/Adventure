#include "items/Container.h"

#include <cmath>

namespace adventure
{
	OpenResult tryOpenContainer(Container& c, Inventory& inv, std::vector<Pickup>& pickups)
	{
		if (c.open)
			return OpenResult::AlreadyOpen;
		if (c.locked)
		{
			if (!removeItem(inv, kItemKey, 1)) // spend a key to unlock
				return OpenResult::Locked;
			c.locked = false;
		}
		c.open = true;

		const bool empty = c.contents.empty();
		int i = 0;
		for (const ItemStack& s : c.contents) // spill each stack as one pickup, scattered around the chest
		{
			const float ang = (i % 8) * 0.785f;
			const Vector3 p = {c.position.x + std::cos(ang) * 0.35f, c.position.y, c.position.z + std::sin(ang) * 0.35f};
			pickups.push_back(Pickup{p, s.itemId, true, s.count});
			++i;
		}
		c.contents.clear();
		return empty ? OpenResult::Empty : OpenResult::Opened;
	}

	int nearestContainer(const std::vector<Container>& containers, Vector3 playerPos, float playerYaw, float range)
	{
		const float fx = std::sin(playerYaw);
		const float fz = -std::cos(playerYaw);
		int best = -1;
		float bestD = range;
		for (int i = 0; i < (int)containers.size(); ++i)
		{
			const Container& c = containers[i];
			if (c.open)
				continue;
			const float dx = c.position.x - playerPos.x;
			const float dz = c.position.z - playerPos.z;
			const float d = std::sqrt(dx * dx + dz * dz);
			if (d > range)
				continue;
			if (d > 0.0001f && (fx * dx + fz * dz) / d < 0.35f) // must be roughly in front (~70 deg)
				continue;
			if (d < bestD)
			{
				bestD = d;
				best = i;
			}
		}
		return best;
	}

	void resolveActorContainers(Vector3& pos, float radius, float height, const std::vector<Container>& containers)
	{
		for (const Container& c : containers)
		{
			if (pos.y - height * 0.5f >= c.position.y + c.height * 0.5f || pos.y + height * 0.5f <= c.position.y - c.height * 0.5f)
				continue;
			const float dx = pos.x - c.position.x;
			const float dz = pos.z - c.position.z;
			const float minD = radius + c.radius;
			const float d = std::sqrt(dx * dx + dz * dz);
			if (d >= minD)
				continue;
			if (d > 0.0001f)
			{
				const float push = minD - d;
				pos.x += (dx / d) * push;
				pos.z += (dz / d) * push;
			}
			else
			{
				pos.x += minD;
			}
		}
	}
} // namespace adventure
