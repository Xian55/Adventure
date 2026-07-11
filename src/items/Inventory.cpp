#include "items/Inventory.h"

#include <algorithm>

namespace adventure
{
	int itemCount(const Inventory& inv, int itemId)
	{
		int n = 0;
		for (const ItemStack& s : inv.slots)
			if (s.itemId == itemId)
				n += s.count;
		return n;
	}

	int addItem(Inventory& inv, int itemId, int count)
	{
		if (itemId == kItemNone || count <= 0)
			return 0;
		const ItemDef& def = itemDef(itemId);
		const int maxStack = def.maxStack > 0 ? def.maxStack : 1;
		int added = 0;

		for (ItemStack& s : inv.slots) // top up existing stacks
		{
			if (s.itemId != itemId || s.count >= maxStack)
				continue;
			const int take = std::min(maxStack - s.count, count - added);
			s.count += take;
			added += take;
			if (added >= count)
				return added;
		}
		while (added < count && (int)inv.slots.size() < inv.capacity) // open new stacks
		{
			const int take = std::min(maxStack, count - added);
			inv.slots.push_back(ItemStack{itemId, take});
			added += take;
		}
		return added;
	}

	bool removeItem(Inventory& inv, int itemId, int count)
	{
		if (count <= 0)
			return true;
		if (itemCount(inv, itemId) < count)
			return false;
		for (auto it = inv.slots.begin(); it != inv.slots.end() && count > 0;)
		{
			if (it->itemId == itemId)
			{
				const int take = std::min(it->count, count);
				it->count -= take;
				count -= take;
				if (it->count == 0)
				{
					it = inv.slots.erase(it);
					continue;
				}
			}
			++it;
		}
		return true;
	}
} // namespace adventure
