#pragma once

// Item definitions. The item *set* + kinds live here (C++); tunable numbers move to Lua later (needs a
// table-read binding — for now they're constants). Inventory logic is pure -> headless-testable.
namespace adventure
{
	enum class ItemKind
	{
		Consumable, // used on pickup (e.g. heals)
		Coin,       // currency, stacks high
		Key,        // opens locks (M3 chests)
		Weapon,     // equippable (M3/M4)
	};

	// Built-in item ids (0 = nothing).
	enum ItemId
	{
		kItemNone = 0,
		kItemHealthPotion = 1,
		kItemCoin = 2,
		kItemKey = 3,
		kItemSword = 4,
		kItemDagger = 5,
		kItemMace = 6,
	};

	struct ItemDef
	{
		int id = kItemNone;
		const char* name = "None";
		ItemKind kind = ItemKind::Coin;
		int maxStack = 1;
		float value = 0.0f; // Consumable: heal amount; Coin: worth; Weapon: damage (future)
	};

	// Definition for an id (returns the None def for unknown ids).
	const ItemDef& itemDef(int id);
} // namespace adventure
