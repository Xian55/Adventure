#include <doctest/doctest.h>
#include "items/Inventory.h"

using namespace adventure;

TEST_CASE("adding items stacks up to maxStack, then opens new slots")
{
	Inventory inv;              // capacity 16
	addItem(inv, kItemCoin, 5); // coins maxStack 999
	CHECK(itemCount(inv, kItemCoin) == 5);
	CHECK(inv.slots.size() == 1);
	addItem(inv, kItemCoin, 3);
	CHECK(itemCount(inv, kItemCoin) == 8);
	CHECK(inv.slots.size() == 1); // same stack

	addItem(inv, kItemHealthPotion, 12); // potions maxStack 10 -> 10 + 2 across two slots
	CHECK(itemCount(inv, kItemHealthPotion) == 12);
	CHECK(inv.slots.size() == 3);
}

TEST_CASE("capacity caps how much can be added; addItem returns the amount taken")
{
	Inventory inv;
	inv.capacity = 2;
	CHECK(addItem(inv, kItemKey, 10) == 10); // keys maxStack 10 -> one slot
	CHECK(inv.slots.size() == 1);
	CHECK(addItem(inv, kItemKey, 10) == 10); // second slot fills capacity
	CHECK(inv.slots.size() == 2);
	CHECK(addItem(inv, kItemKey, 5) == 0); // full -> nothing added
	CHECK(itemCount(inv, kItemKey) == 20);
}

TEST_CASE("removing items decrements and clears emptied stacks")
{
	Inventory inv;
	addItem(inv, kItemCoin, 8);
	CHECK(removeItem(inv, kItemCoin, 3));
	CHECK(itemCount(inv, kItemCoin) == 5);
	CHECK(removeItem(inv, kItemCoin, 5));
	CHECK(itemCount(inv, kItemCoin) == 0);
	CHECK(inv.slots.empty()); // stack removed when emptied
}

TEST_CASE("removing more than held fails without changing anything")
{
	Inventory inv;
	addItem(inv, kItemCoin, 4);
	CHECK_FALSE(removeItem(inv, kItemCoin, 5));
	CHECK(itemCount(inv, kItemCoin) == 4);
	CHECK_FALSE(removeItem(inv, kItemKey, 1)); // none held
}

TEST_CASE("adding nothing / unknown is a no-op")
{
	Inventory inv;
	CHECK(addItem(inv, kItemNone, 5) == 0);
	CHECK(addItem(inv, kItemCoin, 0) == 0);
	CHECK(inv.slots.empty());
}
