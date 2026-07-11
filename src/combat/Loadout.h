#pragma once
#include "combat/Melee.h"
#include "items/Inventory.h"

// Equip / weapon-swap. Maps weapon items to their WeaponDef and cycles through the weapons the player holds.
// Pure logic (no raylib window) -> headless-testable.
namespace adventure
{
	bool isWeapon(int itemId);
	const char* weaponName(int itemId);

	// WeaponDef for a weapon item. The Sword uses the Lua-tuned `swordDef`; others are derived presets
	// (dagger = fast/short/light, mace = slow/heavy/knockback). Unknown ids fall back to the sword.
	WeaponDef weaponDefFor(int itemId, const WeaponDef& swordDef);

	// The next weapon item the player holds after `current` (cycles by item id). Returns `current` if it's
	// the only one held, the first held weapon if `current` isn't held, or kItemNone if none are held. Pure.
	int nextWeapon(const Inventory& inv, int current);
} // namespace adventure
