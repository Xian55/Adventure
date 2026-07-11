-- Item definitions (name + numbers). Hot-reloadable: C++ reads these, so item stats are tuned here.
-- The item *set* and each item's kind (Consumable/Coin/Key/Weapon) live in C++ (code branches on kind).
items = {
	health_potion = { name = "Health Potion", stack = 10,  value = 25 }, -- value = heal amount
	coin          = { name = "Coin",          stack = 999, value = 1 },
	key           = { name = "Key",           stack = 10,  value = 0 },
	sword         = { name = "Sword",         stack = 1,   value = 25 }, -- value = base damage (display)
	dagger        = { name = "Dagger",        stack = 1,   value = 15 },
	mace          = { name = "Mace",          stack = 1,   value = 40 },
}

print(string.format("items loaded: potion heals %d, mace dmg %d", items.health_potion.value, items.mace.value))
