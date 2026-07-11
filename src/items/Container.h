#pragma once
#include "raylib.h"
#include "items/Inventory.h"
#include "items/Pickup.h"

#include <vector>

// Chests: interactable containers that hold items and may be locked (need a key). Pure logic
// (no raylib window) -> headless-testable. Opening spills the contents into the world as pickups.
namespace adventure
{
	struct Container
	{
		Vector3 position{0, 0, 0}; // AABB center, engine space
		float radius = 0.5f;
		float height = 0.8f;
		int lockId = 0; // 0 = unlocked; > 0 needs a key (single key type for now — id match is future)
		bool locked = false;
		bool open = false;
		std::vector<ItemStack> contents;
	};

	enum class OpenResult
	{
		Opened,      // opened this call; contents spilled
		Locked,      // needs a key the player doesn't have
		AlreadyOpen, // was already open
		Empty,       // opened, but held nothing
	};

	// Try to open a container: a locked one consumes one key from `inv` (fails with Locked if none), unless
	// `lockpick` is set (opens locks for free). On opening, spills its contents as Pickups. Pure.
	OpenResult tryOpenContainer(Container& c, Inventory& inv, std::vector<Pickup>& pickups, bool lockpick = false);

	// Index of the closest openable container within `range` and in front of the player (dot test), or -1.
	// Skips already-open chests. Pure.
	int nearestContainer(const std::vector<Container>& containers, Vector3 playerPos, float playerYaw, float range);

	// Push an actor cylinder out of every container it overlaps (chests are solid, open or closed). Pure.
	void resolveActorContainers(Vector3& pos, float radius, float height, const std::vector<Container>& containers);
} // namespace adventure
