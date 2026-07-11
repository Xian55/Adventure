#include "items/Item.h"

namespace adventure
{
	const ItemDef& itemDef(int id)
	{
		static const ItemDef kDefs[] = {
		    {kItemNone, "None", ItemKind::Coin, 0, 0.0f},
		    {kItemHealthPotion, "Health Potion", ItemKind::Consumable, 10, 25.0f},
		    {kItemCoin, "Coin", ItemKind::Coin, 999, 1.0f},
		    {kItemKey, "Key", ItemKind::Key, 10, 0.0f},
		};
		for (const ItemDef& d : kDefs)
			if (d.id == id)
				return d;
		return kDefs[0];
	}
} // namespace adventure
