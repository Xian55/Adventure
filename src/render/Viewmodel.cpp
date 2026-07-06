#include "render/Viewmodel.h"

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#include <cmath>

namespace adventure
{
	void drawViewmodel(float bobPhase, float bobAmount, float t)
	{
		Camera3D vcam{};
		vcam.position = Vector3{0, 0, 0};
		vcam.target = Vector3{0, 0, -1};
		vcam.up = Vector3{0, 1, 0};
		vcam.fovy = 55.0f;
		vcam.projection = CAMERA_PERSPECTIVE;

		const float bobY = sinf(bobPhase) * bobAmount;
		const float bobX = cosf(bobPhase * 0.5f) * bobAmount * 0.5f;
		const Color skin = Color{205, 160, 120, 255};
		const Color skinDk = Color{150, 110, 80, 255};

		BeginMode3D(vcam);
		rlDisableDepthTest(); // viewmodel always draws over the world

		// Torch (left hand): vertical wooden shaft in a fist, tall flame on top.
		{
			rlPushMatrix();
			rlTranslatef(-0.42f + bobX, -0.3f + bobY, -0.8f);
			rlRotatef(-6.0f, 0, 0, 1);
			DrawCube(Vector3{0, 0, 0}, 0.05f, 0.55f, 0.05f, Color{90, 62, 38, 255}); // shaft
			DrawCubeWires(Vector3{0, 0, 0}, 0.05f, 0.55f, 0.05f, Color{55, 38, 24, 255});
			DrawCube(Vector3{0, -0.12f, 0.015f}, 0.13f, 0.15f, 0.13f, skin); // fist
			DrawCubeWires(Vector3{0, -0.12f, 0.015f}, 0.13f, 0.15f, 0.13f, skinDk);
			const float fl = 0.85f + 0.15f * sinf(t * 33.0f) * cosf(t * 19.0f);
			const float wob = 0.012f * sinf(t * 21.0f);
			DrawCylinderEx(Vector3{0, 0.28f, 0}, Vector3{wob, 0.28f + 0.34f * fl, 0}, 0.07f, 0.0f, 8, Color{235, 120, 30, 255});          // outer flame
			DrawCylinderEx(Vector3{0, 0.30f, 0}, Vector3{wob * 0.5f, 0.30f + 0.22f * fl, 0}, 0.038f, 0.0f, 8, Color{255, 220, 120, 255}); // inner flame
			rlPopMatrix();
		}

		// Sword (right hand): fist on the hilt, blade pointing straight up.
		{
			rlPushMatrix();
			rlTranslatef(0.42f + bobX, -0.32f + bobY, -0.8f);
			rlRotatef(5.0f, 0, 0, 1);
			rlRotatef(-6.0f, 1, 0, 0);
			DrawCube(Vector3{0, -0.14f, 0.0f}, 0.16f, 0.12f, 0.15f, Color{110, 112, 122, 255}); // bracer
			DrawCubeWires(Vector3{0, -0.14f, 0.0f}, 0.16f, 0.12f, 0.15f, Color{70, 72, 82, 255});
			DrawCube(Vector3{0, 0.0f, 0.0f}, 0.14f, 0.16f, 0.13f, skin); // fist
			DrawCubeWires(Vector3{0, 0.0f, 0.0f}, 0.14f, 0.16f, 0.13f, skinDk);
			DrawCube(Vector3{0, 0.1f, 0}, 0.06f, 0.06f, 0.06f, Color{160, 130, 75, 255});   // pommel-up cap
			DrawCube(Vector3{0, 0.18f, 0}, 0.28f, 0.045f, 0.05f, Color{150, 120, 70, 255}); // crossguard
			DrawCube(Vector3{0, 0.62f, 0}, 0.05f, 0.85f, 0.02f, Color{182, 196, 188, 255}); // blade
			DrawCubeWires(Vector3{0, 0.62f, 0}, 0.05f, 0.85f, 0.02f, Color{92, 108, 102, 255});
			rlPopMatrix();
		}

		rlEnableDepthTest();
		EndMode3D();
	}
} // namespace adventure
