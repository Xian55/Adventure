#include <doctest/doctest.h>
#include "items/Pickup.h"

using namespace adventure;

TEST_CASE("a consumable pickup is used on the spot (heals, clamped), not bagged")
{
	Inventory inv;
	std::vector<Pickup> pickups{Pickup{{0, 0.5f, 0}, kItemHealthPotion, true}};
	float hp = 60.0f;
	collectPickups(pickups, Vector3{0, 0.5f, 0.3f}, inv, hp, 100.0f, 1.0f);
	CHECK(hp == doctest::Approx(85.0)); // +25 heal
	CHECK_FALSE(pickups[0].active);
	CHECK(inv.slots.empty()); // consumed, not stored

	pickups = {Pickup{{0, 0.5f, 0}, kItemHealthPotion, true}};
	hp = 90.0f;
	collectPickups(pickups, Vector3{0, 0.5f, 0}, inv, hp, 100.0f, 1.0f);
	CHECK(hp == doctest::Approx(100.0)); // clamped, not 115
}

TEST_CASE("a non-consumable pickup goes into the inventory")
{
	Inventory inv;
	std::vector<Pickup> pickups{
	    Pickup{{0, 0.5f, 0}, kItemCoin, true},
	    Pickup{{0, 0.5f, 0}, kItemKey, true},
	};
	float hp = 50.0f;
	collectPickups(pickups, Vector3{0, 0.5f, 0}, inv, hp, 100.0f, 1.0f);
	CHECK(hp == doctest::Approx(50.0)); // no heal
	CHECK(itemCount(inv, kItemCoin) == 1);
	CHECK(itemCount(inv, kItemKey) == 1);
	CHECK_FALSE(pickups[0].active);
	CHECK_FALSE(pickups[1].active);
}

TEST_CASE("a distant pickup is left alone")
{
	Inventory inv;
	std::vector<Pickup> pickups{Pickup{{0, 0.5f, 0}, kItemCoin, true}};
	float hp = 50.0f;
	collectPickups(pickups, Vector3{5, 0.5f, 0}, inv, hp, 100.0f, 1.0f);
	CHECK(itemCount(inv, kItemCoin) == 0);
	CHECK(pickups[0].active);
}

TEST_CASE("a full inventory leaves the pickup in the world")
{
	Inventory inv;
	inv.capacity = 0; // no room
	std::vector<Pickup> pickups{Pickup{{0, 0.5f, 0}, kItemCoin, true}};
	float hp = 50.0f;
	collectPickups(pickups, Vector3{0, 0.5f, 0}, inv, hp, 100.0f, 1.0f);
	CHECK(pickups[0].active); // couldn't pick it up
	CHECK(itemCount(inv, kItemCoin) == 0);
}
