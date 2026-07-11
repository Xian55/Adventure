#pragma once
#include "items/Item.h"

#include <vector>

// A slot-based inventory: stacks of items up to each item's maxStack, bounded by a slot capacity.
// Pure logic (no raylib) -> headless-testable.
namespace adventure
{
	struct ItemStack
	{
		int itemId = kItemNone;
		int count = 0;
	};

	struct Inventory
	{
		std::vector<ItemStack> slots;
		int capacity = 16; // max number of stacks
	};

	// Add up to `count` of an item: tops up existing stacks first, then opens new slots, respecting each
	// item's maxStack and the slot capacity. Returns how many were actually added (< count if it filled up).
	int addItem(Inventory& inv, int itemId, int count);

	// Remove `count` of an item if the inventory holds at least that many; else no-op. Returns success.
	bool removeItem(Inventory& inv, int itemId, int count);

	int itemCount(const Inventory& inv, int itemId); // total across all stacks
} // namespace adventure
