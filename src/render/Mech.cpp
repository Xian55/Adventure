#include "render/Mech.h"

#include "raylib.h"
#include "rlgl.h"

namespace adventure
{
	void drawDoors(const std::vector<Door>& doors)
	{
		for (const Door& d : doors)
		{
			const Vector3 c = doorCenter(d);
			const Color wood = d.locked ? Color{92, 70, 52, 255} : Color{112, 86, 60, 255};
			DrawCube(c, d.half.x * 2.0f, d.half.y * 2.0f, d.half.z * 2.0f, wood);
			DrawCubeWires(c, d.half.x * 2.0f, d.half.y * 2.0f, d.half.z * 2.0f, Color{60, 45, 32, 255});
			if (d.locked) // lock plate on the face
				DrawCube(Vector3{c.x, c.y, c.z + d.half.z + 0.03f}, 0.16f, 0.16f, 0.05f, Color{170, 172, 182, 255});
		}
	}

	void drawLevers(const std::vector<Lever>& levers)
	{
		for (const Lever& l : levers)
		{
			DrawCube(Vector3{l.position.x, l.position.y + 0.12f, l.position.z}, 0.22f, 0.24f, 0.22f, Color{78, 80, 92, 255}); // housing
			DrawCubeWires(Vector3{l.position.x, l.position.y + 0.12f, l.position.z}, 0.22f, 0.24f, 0.22f, Color{40, 42, 50, 255});
			rlPushMatrix();
			rlTranslatef(l.position.x, l.position.y + 0.22f, l.position.z);
			rlRotatef(l.on ? 42.0f : -42.0f, 1, 0, 0); // handle throws forward/back
			DrawCube(Vector3{0, 0.26f, 0}, 0.06f, 0.5f, 0.06f, Color{62, 52, 46, 255});
			DrawCube(Vector3{0, 0.5f, 0}, 0.1f, 0.1f, 0.1f, l.on ? Color{90, 210, 110, 255} : Color{205, 70, 55, 255}); // knob (green=on)
			rlPopMatrix();
		}
	}

	void drawPlates(const std::vector<Plate>& plates)
	{
		for (const Plate& p : plates)
		{
			const float sink = p.pressed ? -0.05f : 0.0f;
			const Color top = p.pressed ? Color{130, 130, 95, 255} : Color{104, 104, 116, 255};
			DrawCube(Vector3{p.position.x, p.position.y + sink, p.position.z}, p.half.x * 2.0f, p.half.y * 2.0f, p.half.z * 2.0f, top);
			DrawCubeWires(Vector3{p.position.x, p.position.y + sink, p.position.z}, p.half.x * 2.0f, p.half.y * 2.0f, p.half.z * 2.0f, Color{48, 48, 58, 255});
		}
	}
} // namespace adventure
