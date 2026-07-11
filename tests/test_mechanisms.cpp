#include <doctest/doctest.h>
#include "mech/Mechanisms.h"

using namespace adventure;

namespace
{
	void run(std::vector<Door>& doors, float total)
	{
		const float dt = 1.0f / 60.0f;
		for (float e = 0.0f; e < total; e += dt)
			updateDoors(doors, dt);
	}
} // namespace

TEST_CASE("a door eases open when wanted and closes when not; the solid box moves with it")
{
	std::vector<Door> doors(1);
	doors[0].position = {0, 1.5f, 0};
	doors[0].openMove = {0, 3.0f, 0};
	doors[0].openTime = 0.5f;

	doors[0].wantOpen = true;
	run(doors, 0.6f);
	CHECK(doors[0].t == doctest::Approx(1.0));
	CHECK(doorSolid(doors[0]).center.y == doctest::Approx(4.5)); // slid up by openMove

	doors[0].wantOpen = false;
	run(doors, 0.6f);
	CHECK(doors[0].t == doctest::Approx(0.0));
	CHECK(doorCenter(doors[0]).y == doctest::Approx(1.5));
}

TEST_CASE("a lever or plate opens the door it targets")
{
	std::vector<Door> doors(1);
	doors[0].targetname = "gate";
	std::vector<Lever> levers(1);
	levers[0].target = "gate";
	std::vector<Plate> plates(1);
	plates[0].target = "other";

	applyActivations(doors, levers, plates);
	CHECK_FALSE(doors[0].wantOpen); // lever off, plate targets something else

	levers[0].on = true;
	applyActivations(doors, levers, plates);
	CHECK(doors[0].wantOpen);

	levers[0].on = false;
	plates[0].target = "gate";
	plates[0].pressed = true;
	applyActivations(doors, levers, plates);
	CHECK(doors[0].wantOpen); // plate now holds it
}

TEST_CASE("a plate presses when an occupant stands on its pad")
{
	std::vector<Plate> plates(1);
	plates[0].position = {2, 0.1f, 0};
	plates[0].half = {0.6f, 0.15f, 0.6f};

	updatePlates(plates, {Vector3{5, 0.1f, 0}}); // off the pad
	CHECK_FALSE(plates[0].pressed);

	updatePlates(plates, {Vector3{2.2f, 0.0f, 0.1f}}); // on the pad
	CHECK(plates[0].pressed);
}

TEST_CASE("a locked door needs a key and ignores activators until unlocked")
{
	std::vector<Door> doors(1);
	doors[0].locked = true;
	doors[0].lockId = 1;
	doors[0].targetname = "cell";
	std::vector<Lever> levers(1);
	levers[0].target = "cell";
	levers[0].on = true;
	std::vector<Plate> plates;

	applyActivations(doors, levers, plates);
	CHECK_FALSE(doors[0].wantOpen); // locked -> lever can't open it

	Inventory inv;
	CHECK(useDoor(doors[0], inv) == DoorUse::Locked); // no key
	CHECK(doors[0].locked);

	addItem(inv, kItemKey, 1);
	CHECK(useDoor(doors[0], inv) == DoorUse::Opened); // key spent
	CHECK_FALSE(doors[0].locked);
	CHECK(itemCount(inv, kItemKey) == 0);
	applyActivations(doors, levers, plates);
	CHECK(doors[0].wantOpen);
}

TEST_CASE("a plain door toggles on use; nearestDoor skips activator-driven doors")
{
	std::vector<Door> doors(2);
	doors[0].position = {0, 1, -1}; // plain, in front
	doors[1].position = {0, 1, -1.5f};
	doors[1].targetname = "remote"; // lever-driven -> not E-usable

	CHECK(nearestDoor(doors, Vector3{0, 1, 0}, 0.0f, 3.0f) == 0);

	Inventory inv;
	CHECK(useDoor(doors[0], inv) == DoorUse::Toggled);
	CHECK(doors[0].manualOpen);
	CHECK(useDoor(doors[0], inv) == DoorUse::Toggled);
	CHECK_FALSE(doors[0].manualOpen);
}
