#pragma once
#include "raylib.h"
#include "items/Collide.h"
#include "items/Inventory.h"

#include <string>
#include <vector>

// Doors + activators (levers, pressure plates), wired by target/targetname (Quake-style). A door opens
// while any linked lever is on or plate is pressed; it can also be key-locked or a plain E-toggle door.
// Pure logic (no raylib window) -> headless-testable.
namespace adventure
{
	struct Door
	{
		Vector3 position{0, 0, 0}; // solid box center when closed
		Vector3 half{1.0f, 1.5f, 0.2f};
		Vector3 openMove{0, 3.0f, 0}; // slides here when fully open (e.g. up into the ceiling)
		std::string targetname;       // activator-driven if set
		int lockId = 0;               // > 0 = needs a key (single key type for now)
		bool locked = false;
		bool manualOpen = false; // latched open by an E-use / after unlocking
		float openTime = 0.8f;
		float t = 0.0f; // 0 closed .. 1 open (animated)
		bool wantOpen = false;
	};

	struct Lever
	{
		Vector3 position{0, 0, 0};
		float radius = 0.7f; // interact distance
		std::string target;
		bool on = false;
	};

	struct Plate
	{
		Vector3 position{0, 0, 0}; // pad center
		Vector3 half{0.6f, 0.15f, 0.6f};
		std::string target;
		bool pressed = false;
	};

	enum class DoorUse
	{
		Opened,  // a locked door unlocked (key spent) and opened
		Locked,  // locked and no key held
		Toggled, // a plain door toggled open/closed
	};

	Vector3 doorCenter(const Door& d);                    // animated center (eased)
	SolidBox doorSolid(const Door& d);                    // collision box at the animated center
	void updateDoors(std::vector<Door>& doors, float dt); // ease t toward wantOpen

	// Press a plate if any occupant point (actor/prop feet) is over its pad.
	void updatePlates(std::vector<Plate>& plates, const std::vector<Vector3>& occupants);

	// Set each door's wantOpen = manualOpen OR any linked lever(on)/plate(pressed) — unless it is still locked.
	void applyActivations(std::vector<Door>& doors, const std::vector<Lever>& levers, const std::vector<Plate>& plates);

	// E-use a door: a locked one spends a key from `inv` (Locked if none), unless `lockpick` is set, then
	// latches open; a plain door (no targetname, unlocked) toggles. Pure.
	DoorUse useDoor(Door& d, Inventory& inv, bool lockpick = false);

	int nearestLever(const std::vector<Lever>& levers, Vector3 playerPos, float playerYaw, float range);
	// Nearest facing E-usable door (locked, or plain with no targetname), or -1.
	int nearestDoor(const std::vector<Door>& doors, Vector3 playerPos, float playerYaw, float range);
} // namespace adventure
