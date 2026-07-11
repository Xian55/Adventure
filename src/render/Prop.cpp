#include "render/Prop.h"

#include <cmath>

namespace adventure
{
	namespace
	{
		void drawBarrel(const Destructible& p, Color body, Color band)
		{
			const Vector3 base = {p.position.x, p.position.y - p.height * 0.5f, p.position.z};
			DrawCylinder(base, p.radius, p.radius, p.height, 12, body);
			DrawCylinderWires(base, p.radius, p.radius, p.height, 12, band);
			for (float f : {0.28f, 0.72f}) // two hoop bands
			{
				const Vector3 a = {base.x, base.y + p.height * f - 0.03f, base.z};
				const Vector3 b = {base.x, base.y + p.height * f + 0.03f, base.z};
				DrawCylinderEx(a, b, p.radius * 1.04f, p.radius * 1.04f, 12, band);
			}
		}

		void drawDebris(const Destructible& p, float time)
		{
			const float life = fmaxf(0.0f, p.breakTimer);
			const float sz = 0.12f * fminf(1.0f, life / 0.5f);
			if (sz <= 0.001f)
				return;
			const Color c = Color{110, 85, 55, 255};
			for (int i = 0; i < 5; ++i)
			{
				const float ang = i * 1.2566f;                     // 72 deg apart
				const float rad = p.radius * (1.0f - life * 1.4f); // scatter outward as it fades
				const Vector3 d = {
				    p.position.x + std::cos(ang) * rad,
				    p.position.y - p.height * 0.4f + std::sin(time * 3.0f + i) * 0.02f,
				    p.position.z + std::sin(ang) * rad,
				};
				DrawCube(d, sz, sz, sz, c);
			}
		}
	} // namespace

	void drawProps(const std::vector<Destructible>& props, float time)
	{
		for (const Destructible& p : props)
		{
			if (!p.active)
				continue;
			if (p.broken)
			{
				drawDebris(p, time);
				continue;
			}
			switch (p.kind)
			{
			case PropKind::Crate:
				DrawCube(p.position, p.radius * 2.0f, p.height, p.radius * 2.0f, Color{135, 100, 60, 255});
				DrawCubeWires(p.position, p.radius * 2.0f, p.height, p.radius * 2.0f, Color{80, 60, 35, 255});
				break;
			case PropKind::Keg:
				drawBarrel(p, Color{150, 120, 75, 255}, Color{95, 75, 50, 255});
				break;
			case PropKind::Barrel:
			default:
				drawBarrel(p, Color{120, 80, 45, 255}, Color{70, 55, 45, 255});
				break;
			}
		}
	}

	void drawPickups(const std::vector<Pickup>& pickups, float time)
	{
		for (const Pickup& pk : pickups)
		{
			if (!pk.active)
				continue;
			const float bob = std::sin(time * 3.0f) * 0.08f;
			const Vector3 pos = {pk.position.x, pk.position.y + 0.25f + bob, pk.position.z};
			Color c;
			switch (itemDef(pk.itemId).kind)
			{
			case ItemKind::Consumable:
				c = Color{80, 220, 120, 255}; // green orb
				break;
			case ItemKind::Coin:
				c = Color{235, 200, 70, 255}; // gold
				break;
			case ItemKind::Key:
				c = Color{200, 210, 225, 255}; // pale steel
				break;
			default:
				c = Color{220, 210, 120, 255};
				break;
			}
			DrawCube(pos, 0.18f, 0.18f, 0.18f, c);
			DrawCubeWires(pos, 0.2f, 0.2f, 0.2f, Color{255, 255, 255, 200});
		}
	}
} // namespace adventure
