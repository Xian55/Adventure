#pragma once
#include <string>

// Item definitions. The item *set* + kinds live in C++ (code branches on kind); names + numbers (maxStack,
// value) are overridable from scripts/items.lua via setItemDef. Inventory logic is pure -> headless-testable.
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
		kItemCrossbow = 7,
	};

	struct ItemDef
	{
		int id = kItemNone;
		std::string name = "None";
		ItemKind kind = ItemKind::Coin;
		int maxStack = 1;
		float value = 0.0f; // Consumable: heal amount; Coin: worth; Weapon: damage
	};

	// Definition for an id (returns the None def for unknown ids).
	const ItemDef& itemDef(int id);

	// Override the data fields for an id (name/maxStack/value) — kind stays fixed in code. Used by `main`
	// to apply scripts/items.lua. Unknown ids are ignored.
	void setItemDef(int id, const std::string& name, int maxStack, float value);
} // namespace adventure
