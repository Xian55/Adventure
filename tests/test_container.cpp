#include <doctest/doctest.h>
#include "items/Container.h"

#include <cmath>

using namespace adventure;

namespace
{
	Container chest(bool locked, std::vector<ItemStack> contents)
	{
		Container c;
		c.position = {0, 0.5f, 0};
		c.locked = locked;
		c.lockId = locked ? 1 : 0;
		c.contents = std::move(contents);
		return c;
	}
} // namespace

TEST_CASE("opening an unlocked chest spills its contents as pickups")
{
	Inventory inv;
	std::vector<Pickup> pickups;
	Container c = chest(false, {{kItemCoin, 10}, {kItemHealthPotion, 1}});
	OpenResult r = tryOpenContainer(c, inv, pickups);
	CHECK(r == OpenResult::Opened);
	CHECK(c.open);
	REQUIRE(pickups.size() == 2);
	CHECK(pickups[0].itemId == kItemCoin);
	CHECK(pickups[0].count == 10); // stack spilled as one pickup worth 10
	CHECK(c.contents.empty());
}

TEST_CASE("a locked chest needs a key; opening spends one")
{
	Inventory inv;
	std::vector<Pickup> pickups;
	Container c = chest(true, {{kItemCoin, 5}});

	CHECK(tryOpenContainer(c, inv, pickups) == OpenResult::Locked); // no key
	CHECK(c.locked);
	CHECK_FALSE(c.open);
	CHECK(pickups.empty());

	addItem(inv, kItemKey, 1);
	CHECK(tryOpenContainer(c, inv, pickups) == OpenResult::Opened); // key spent
	CHECK_FALSE(c.locked);
	CHECK(c.open);
	CHECK(itemCount(inv, kItemKey) == 0);
	CHECK(pickups.size() == 1);
}

TEST_CASE("opening an already-open or empty chest")
{
	Inventory inv;
	std::vector<Pickup> pickups;

	Container empty = chest(false, {});
	CHECK(tryOpenContainer(empty, inv, pickups) == OpenResult::Empty);
	CHECK(empty.open);
	CHECK(pickups.empty());

	CHECK(tryOpenContainer(empty, inv, pickups) == OpenResult::AlreadyOpen);
}

TEST_CASE("nearestContainer picks the closest one in front and skips open ones")
{
	std::vector<Container> cs(3);
	cs[0].position = {0, 0.5f, -1.0f}; // in front, near
	cs[1].position = {0, 0.5f, -3.0f}; // in front, far
	cs[2].position = {0, 0.5f, 2.0f};  // behind
	CHECK(nearestContainer(cs, Vector3{0, 0.5f, 0}, 0.0f, 4.0f) == 0);

	cs[0].open = true; // now the next-nearest in front wins
	CHECK(nearestContainer(cs, Vector3{0, 0.5f, 0}, 0.0f, 4.0f) == 1);

	CHECK(nearestContainer(cs, Vector3{0, 0.5f, 0}, 0.0f, 0.5f) == -1); // nothing in range
}

TEST_CASE("a chest blocks an actor")
{
	std::vector<Container> cs(1);
	cs[0].position = {0, 0.4f, 0};
	cs[0].radius = 0.5f;
	cs[0].height = 0.8f;
	Vector3 pos = {0.6f, 0.4f, 0.0f}; // overlapping (0.6 < 0.3 + 0.5)
	resolveActorContainers(pos, 0.3f, 1.8f, cs);
	const float d = std::sqrt(pos.x * pos.x + pos.z * pos.z);
	CHECK(d == doctest::Approx(0.8).epsilon(0.01));
}
