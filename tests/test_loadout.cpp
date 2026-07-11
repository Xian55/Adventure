#include <doctest/doctest.h>
#include "combat/Loadout.h"
#include "items/Item.h"

using namespace adventure;

TEST_CASE("weapon items are weapons; others are not")
{
	CHECK(isWeapon(kItemSword));
	CHECK(isWeapon(kItemDagger));
	CHECK(isWeapon(kItemMace));
	CHECK_FALSE(isWeapon(kItemCoin));
	CHECK_FALSE(isWeapon(kItemKey));
}

TEST_CASE("weaponDefFor gives each weapon distinct feel; sword keeps its tuned def")
{
	WeaponDef sword;
	sword.damage = 25.0f;
	sword.reach = 1.9f;

	CHECK(weaponDefFor(kItemSword, sword).damage == doctest::Approx(25.0)); // passthrough
	const WeaponDef dagger = weaponDefFor(kItemDagger, sword);
	const WeaponDef mace = weaponDefFor(kItemMace, sword);
	CHECK(dagger.damage < sword.damage); // lighter
	CHECK(dagger.reach < sword.reach);   // shorter
	CHECK(mace.damage > sword.damage);   // heavier
	CHECK(mace.knockback > dagger.knockback);
	CHECK(mace.recovery > dagger.recovery); // slower
}

TEST_CASE("nextWeapon cycles the weapons the player holds")
{
	Inventory inv;
	CHECK(nextWeapon(inv, kItemSword) == kItemNone); // holds none

	addItem(inv, kItemSword, 1);
	addItem(inv, kItemMace, 1);
	addItem(inv, kItemCoin, 5); // non-weapon ignored

	// held weapons sorted by id: Sword(4), Mace(6)
	CHECK(nextWeapon(inv, kItemSword) == kItemMace);
	CHECK(nextWeapon(inv, kItemMace) == kItemSword); // wraps

	// current not held -> first held weapon
	CHECK(nextWeapon(inv, kItemDagger) == kItemSword);
}

TEST_CASE("a single held weapon cycles to itself")
{
	Inventory inv;
	addItem(inv, kItemDagger, 1);
	CHECK(nextWeapon(inv, kItemDagger) == kItemDagger);
}
