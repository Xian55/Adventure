#include "items/Pickup.h"

namespace adventure
{
	void collectPickups(std::vector<Pickup>& pickups, Vector3 playerPos, Inventory& inv, float& playerHealth, float maxHealth, float radius)
	{
		const float r2 = radius * radius;
		for (Pickup& pk : pickups)
		{
			if (!pk.active)
				continue;
			const float dx = pk.position.x - playerPos.x;
			const float dy = pk.position.y - playerPos.y;
			const float dz = pk.position.z - playerPos.z;
			if (dx * dx + dy * dy + dz * dz > r2)
				continue;

			const int count = pk.count > 0 ? pk.count : 1;
			const ItemDef& def = itemDef(pk.itemId);
			if (def.kind == ItemKind::Consumable)
			{
				playerHealth += def.value * count;
				if (playerHealth > maxHealth)
					playerHealth = maxHealth;
				pk.active = false;
			}
			else
			{
				const int added = addItem(inv, pk.itemId, count); // into the bag; a full bag leaves the rest
				pk.count -= added;
				if (pk.count <= 0)
					pk.active = false;
			}
		}
	}
} // namespace adventure
