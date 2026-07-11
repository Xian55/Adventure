#include "mech/Mechanisms.h"

#include <cmath>

namespace adventure
{
	namespace
	{
		float ease(float t) // smoothstep
		{
			if (t < 0.0f)
				t = 0.0f;
			if (t > 1.0f)
				t = 1.0f;
			return t * t * (3.0f - 2.0f * t);
		}

		// Nearest index whose position is within range and roughly in front of the player.
		template <typename T, typename Pred>
		int nearestFacing(const std::vector<T>& items, Vector3 p, float yaw, float range, Pred usable)
		{
			const float fx = std::sin(yaw);
			const float fz = -std::cos(yaw);
			int best = -1;
			float bestD = range;
			for (int i = 0; i < (int)items.size(); ++i)
			{
				if (!usable(items[i]))
					continue;
				const float dx = items[i].position.x - p.x;
				const float dz = items[i].position.z - p.z;
				const float d = std::sqrt(dx * dx + dz * dz);
				if (d > range)
					continue;
				if (d > 0.0001f && (fx * dx + fz * dz) / d < 0.35f)
					continue;
				if (d < bestD)
				{
					bestD = d;
					best = i;
				}
			}
			return best;
		}
	} // namespace

	Vector3 doorCenter(const Door& d)
	{
		const float e = ease(d.t);
		return Vector3{d.position.x + d.openMove.x * e, d.position.y + d.openMove.y * e, d.position.z + d.openMove.z * e};
	}

	SolidBox doorSolid(const Door& d)
	{
		return SolidBox{doorCenter(d), d.half};
	}

	void updateDoors(std::vector<Door>& doors, float dt)
	{
		for (Door& d : doors)
		{
			const float step = d.openTime > 0.0001f ? dt / d.openTime : 1.0f;
			d.t += (d.wantOpen ? step : -step);
			if (d.t < 0.0f)
				d.t = 0.0f;
			if (d.t > 1.0f)
				d.t = 1.0f;
		}
	}

	void updatePlates(std::vector<Plate>& plates, const std::vector<Vector3>& occupants)
	{
		for (Plate& p : plates)
		{
			bool on = false;
			for (const Vector3& o : occupants)
			{
				if (std::fabs(o.x - p.position.x) <= p.half.x && std::fabs(o.z - p.position.z) <= p.half.z && std::fabs(o.y - p.position.y) <= 1.0f)
				{
					on = true;
					break;
				}
			}
			p.pressed = on;
		}
	}

	void applyActivations(std::vector<Door>& doors, const std::vector<Lever>& levers, const std::vector<Plate>& plates)
	{
		for (Door& d : doors)
		{
			if (d.locked)
			{
				d.wantOpen = false; // stays shut until a key unlocks it
				continue;
			}
			bool open = d.manualOpen;
			if (!d.targetname.empty())
			{
				for (const Lever& l : levers)
					if (l.on && l.target == d.targetname)
						open = true;
				for (const Plate& pl : plates)
					if (pl.pressed && pl.target == d.targetname)
						open = true;
			}
			d.wantOpen = open;
		}
	}

	DoorUse useDoor(Door& d, Inventory& inv, bool lockpick)
	{
		if (d.locked)
		{
			if (!lockpick && !removeItem(inv, kItemKey, 1))
				return DoorUse::Locked;
			d.locked = false;
			d.manualOpen = true; // swings open once unlocked
			return DoorUse::Opened;
		}
		d.manualOpen = !d.manualOpen; // plain toggle door
		return DoorUse::Toggled;
	}

	int nearestLever(const std::vector<Lever>& levers, Vector3 playerPos, float playerYaw, float range)
	{
		return nearestFacing(levers, playerPos, playerYaw, range, [](const Lever&) { return true; });
	}

	int nearestDoor(const std::vector<Door>& doors, Vector3 playerPos, float playerYaw, float range)
	{
		return nearestFacing(doors, playerPos, playerYaw, range, [](const Door& d) { return d.locked || d.targetname.empty(); });
	}
} // namespace adventure
