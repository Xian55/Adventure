#pragma once
#include "raylib.h"
#include "items/Inventory.h"

#include <vector>

// World loot: a floating item the player collects by walking over it. Consumables are used on the spot;
// everything else goes into the inventory. Pure logic (no raylib window) -> headless-testable.
namespace adventure
{
	struct Pickup
	{
		Vector3 position{0, 0, 0};
		int itemId = kItemNone;
		bool active = true;
		int count = 1; // how many the pickup is worth (a spilled coin-pile, etc.)
	};

	// Collect every active pickup within `radius` of the player: a Consumable is used immediately (heal,
	// clamped to maxHealth); anything else is added to `inv` (a pickup stays if the inventory is full). Pure.
	void collectPickups(std::vector<Pickup>& pickups, Vector3 playerPos, Inventory& inv, float& playerHealth, float maxHealth, float radius);
} // namespace adventure
