#include "combat/Loadout.h"
#include "items/Item.h"

#include <vector>

namespace adventure
{
	bool isWeapon(int itemId)
	{
		return itemDef(itemId).kind == ItemKind::Weapon;
	}

	const char* weaponName(int itemId)
	{
		return itemDef(itemId).name.c_str();
	}

	WeaponDef weaponDefFor(int itemId, const WeaponDef& swordDef)
	{
		WeaponDef d = swordDef;
		switch (itemId)
		{
		case kItemDagger: // fast, short, light — spammy jabs
			d.active = 0.07f;
			d.recovery = 0.14f;
			d.reach = 1.4f;
			d.arc = 1.1f;
			d.damage = 15.0f;
			d.knockback = 3.0f;
			d.chargeMax = 0.35f;
			d.chargeDamageMul = 0.5f;
			break;
		case kItemMace: // slow, heavy, big knockback
			d.active = 0.13f;
			d.recovery = 0.36f;
			d.reach = 2.0f;
			d.arc = 1.5f;
			d.damage = 40.0f;
			d.knockback = 12.0f;
			d.chargeMax = 0.7f;
			d.chargeDamageMul = 0.8f;
			break;
		case kItemSword:
		default:
			break; // the Lua-tuned sword
		}
		return d;
	}

	int nextWeapon(const Inventory& inv, int current)
	{
		std::vector<int> held;
		for (const ItemStack& s : inv.slots)
			if (s.count > 0 && isWeapon(s.itemId))
			{
				bool seen = false;
				for (int id : held)
					if (id == s.itemId)
					{
						seen = true;
						break;
					}
				if (!seen)
					held.push_back(s.itemId);
			}
		if (held.empty())
			return kItemNone;

		// Sort by id for a stable cycle order.
		for (std::size_t i = 0; i + 1 < held.size(); ++i)
			for (std::size_t j = 0; j + 1 < held.size() - i; ++j)
				if (held[j] > held[j + 1])
				{
					const int t = held[j];
					held[j] = held[j + 1];
					held[j + 1] = t;
				}

		int idx = -1;
		for (int i = 0; i < (int)held.size(); ++i)
			if (held[i] == current)
			{
				idx = i;
				break;
			}
		if (idx < 0)
			return held[0]; // current not held -> first
		return held[(idx + 1) % held.size()];
	}
} // namespace adventure
