#include "items/Item.h"

namespace adventure
{
	namespace
	{
		ItemDef kDefs[] = {
		    {kItemNone, "None", ItemKind::Coin, 0, 0.0f},
		    {kItemHealthPotion, "Health Potion", ItemKind::Consumable, 10, 25.0f},
		    {kItemCoin, "Coin", ItemKind::Coin, 999, 1.0f},
		    {kItemKey, "Key", ItemKind::Key, 10, 0.0f},
		    {kItemSword, "Sword", ItemKind::Weapon, 1, 25.0f},
		    {kItemDagger, "Dagger", ItemKind::Weapon, 1, 15.0f},
		    {kItemMace, "Mace", ItemKind::Weapon, 1, 40.0f},
		};
	} // namespace

	const ItemDef& itemDef(int id)
	{
		for (const ItemDef& d : kDefs)
			if (d.id == id)
				return d;
		return kDefs[0];
	}

	void setItemDef(int id, const std::string& name, int maxStack, float value)
	{
		for (ItemDef& d : kDefs)
			if (d.id == id)
			{
				d.name = name;
				d.maxStack = maxStack;
				d.value = value;
				return;
			}
	}
} // namespace adventure
