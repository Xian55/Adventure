#include "render/Viewmodel.h"

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#include <cmath>

namespace adventure
{
	void drawViewmodel(float bobPhase, float bobAmount, float t, int meleePhase, float meleeProgress, int swingDir, float charge, float kick, int weapon)
	{
		Camera3D vcam{};
		vcam.position = Vector3{0, 0, 0};
		vcam.target = Vector3{0, 0, -1};
		vcam.up = Vector3{0, 1, 0};
		vcam.fovy = 55.0f;
		vcam.projection = CAMERA_PERSPECTIVE;

		// Lazy figure-8 footstep sway: side-to-side once per stride, a soft double vertical bounce, and a
		// wrist roll — reads as carrying the weapon, not pumping it straight up and down.
		const float bobX = sinf(bobPhase) * bobAmount;
		const float bobY = sinf(bobPhase * 2.0f) * bobAmount * 0.45f;
		const float bobRoll = sinf(bobPhase) * 2.5f; // degrees
		const Color skin = Color{205, 160, 120, 255};
		const Color skinDk = Color{150, 110, 80, 255};

		BeginMode3D(vcam);
		rlDisableDepthTest(); // viewmodel always draws over the world

		// Torch (left hand): vertical wooden shaft in a fist, tall flame on top.
		{
			rlPushMatrix();
			rlTranslatef(-0.42f + bobX, -0.3f + bobY, -0.8f);
			rlRotatef(-6.0f - bobRoll * 0.6f, 0, 0, 1);
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

		// Directional sword pose: a rest pose, a per-direction cock (wound up during Charge) and a strike
		// (played over Active), returning to rest over Recovery.
		struct Pose
		{
			float rotZ, rotX, rotY, push, rise, slideX; // roll / pitch / yaw + forward, vertical, lateral travel
		};
		auto mix = [](Pose a, Pose b, float t) {
			return Pose{
			    a.rotZ + (b.rotZ - a.rotZ) * t,
			    a.rotX + (b.rotX - a.rotX) * t,
			    a.rotY + (b.rotY - a.rotY) * t,
			    a.push + (b.push - a.push) * t,
			    a.rise + (b.rise - a.rise) * t,
			    a.slideX + (b.slideX - a.slideX) * t,
			};
		};
		const Pose rest{16.0f, -6.0f, -13.0f, 0.0f, 0.0f, 0.0f}; // diagonal sword carry (rolled + yawed across view), not held straight up like a racket
		Pose cock = rest, strike = rest;
		switch (swingDir)
		{
		case 1:                                            // Left slash: wind over the right shoulder, cut down across to the lower-left
			cock = {-40, -4, 30, -0.05f, 0.14f, 0.16f};    // raised right, edge turned out
			strike = {72, -2, -24, 0.12f, -0.06f, -0.18f}; // sweeps left + forward through the target
			break;
		case 2: // Right slash: mirror
			cock = {40, -4, -30, -0.05f, 0.14f, -0.16f};
			strike = {-72, -2, 24, 0.12f, -0.06f, 0.18f};
			break;
		case 3:                                        // Forward thrust: level the point forward, drive it out on push (a stab, not a swing)
			cock = {5, -80, 0, -0.16f, 0.05f, 0.04f};  // blade leveled forward, drawn back beside the body
			strike = {5, -90, 0, 0.52f, -0.02f, 0.0f}; // tip lunges straight away along -Z
			break;
		case 4: // Overhead chop: rear up and back, chop down-forward
			cock = {5, 65, 0, 0, 0.20f, 0.0f};
			strike = {5, -60, 0, 0.06f, -0.05f, 0.0f};
			break;
		default: // Neutral flurry: a compact diagonal cut
			cock = {-34, -4, 26, -0.04f, 0.12f, 0.14f};
			strike = {66, -2, -20, 0.10f, -0.05f, -0.16f};
			break;
		}
		Pose p = rest;
		if (meleePhase == 1) // Charge: wind into the cock pose, then hold
			p = mix(rest, cock, fminf(1.0f, charge * 3.0f + 0.15f));
		else if (meleePhase == 2) // Active: strike
			p = mix(cock, strike, meleeProgress);
		else if (meleePhase == 3) // Recovery: return
			p = mix(strike, rest, meleeProgress);

		// Sword (right hand): fist on the hilt, blade carried diagonally across the view; pose drives the swing.
		{
			rlPushMatrix();
			rlTranslatef(0.42f + bobX + p.slideX, -0.32f + bobY + p.rise, -0.8f - p.push);
			rlRotatef(p.rotZ + bobRoll, 0, 0, 1);
			rlRotatef(p.rotY, 0, 1, 0);
			rlRotatef(p.rotX, 1, 0, 0);
			DrawCube(Vector3{0, -0.14f, 0.0f}, 0.16f, 0.12f, 0.15f, Color{110, 112, 122, 255}); // bracer
			DrawCubeWires(Vector3{0, -0.14f, 0.0f}, 0.16f, 0.12f, 0.15f, Color{70, 72, 82, 255});
			DrawCube(Vector3{0, 0.0f, 0.0f}, 0.14f, 0.16f, 0.13f, skin); // fist
			DrawCubeWires(Vector3{0, 0.0f, 0.0f}, 0.14f, 0.16f, 0.13f, skinDk);
			DrawCube(Vector3{0, 0.11f, 0}, 0.045f, 0.07f, 0.05f, Color{160, 130, 75, 255}); // grip collar
			const Color steel = Color{182, 196, 188, 255};
			const Color steelDk = Color{92, 108, 102, 255};
			if (weapon == 1) // Dagger: short stabby blade
			{
				DrawCube(Vector3{0, 0.16f, 0}, 0.13f, 0.035f, 0.05f, Color{150, 120, 70, 255}); // small crossguard
				DrawCube(Vector3{0, 0.4f, 0}, 0.05f, 0.42f, 0.014f, steel);                     // short blade
				DrawCubeWires(Vector3{0, 0.4f, 0}, 0.05f, 0.42f, 0.014f, steelDk);
				DrawCylinderEx(Vector3{0, 0.61f, 0}, Vector3{0, 0.74f, 0}, 0.026f, 0.0f, 4, steel); // point
			}
			else if (weapon == 2) // Mace: wooden haft + a spiked metal head
			{
				const Color head = Color{120, 122, 132, 255};
				DrawCube(Vector3{0, 0.34f, 0}, 0.06f, 0.55f, 0.06f, Color{78, 60, 42, 255}); // haft
				DrawCube(Vector3{0, 0.66f, 0}, 0.2f, 0.2f, 0.2f, head);                      // head
				DrawCubeWires(Vector3{0, 0.66f, 0}, 0.2f, 0.2f, 0.2f, Color{60, 62, 72, 255});
				DrawCube(Vector3{0, 0.8f, 0}, 0.05f, 0.09f, 0.05f, Color{160, 162, 172, 255});      // top spike
				DrawCube(Vector3{0.14f, 0.66f, 0}, 0.09f, 0.05f, 0.05f, Color{160, 162, 172, 255}); // side spikes
				DrawCube(Vector3{-0.14f, 0.66f, 0}, 0.09f, 0.05f, 0.05f, Color{160, 162, 172, 255});
			}
			else if (weapon == 3) // Crossbow: horizontal bow across a stock
			{
				const Color wood = Color{95, 72, 48, 255};
				const Color metal = Color{120, 122, 132, 255};
				DrawCube(Vector3{0, 0.14f, 0}, 0.06f, 0.36f, 0.07f, wood);                   // stock (points forward = up in local)
				DrawCube(Vector3{0, 0.30f, 0}, 0.5f, 0.05f, 0.05f, wood);                    // bow limbs (across)
				DrawCube(Vector3{0, 0.30f, 0}, 0.04f, 0.04f, 0.14f, metal);                  // string bridge / bolt groove
				DrawCube(Vector3{0, 0.36f, 0}, 0.04f, 0.02f, 0.18f, Color{58, 48, 38, 255}); // loaded bolt
			}
			else // Sword (default): crossguard + tapered blade
			{
				DrawCube(Vector3{0, 0.17f, 0}, 0.19f, 0.04f, 0.055f, Color{150, 120, 70, 255}); // crossguard
				DrawCube(Vector3{0, 0.62f, 0}, 0.055f, 0.86f, 0.016f, steel);                   // blade body
				DrawCubeWires(Vector3{0, 0.62f, 0}, 0.055f, 0.86f, 0.016f, steelDk);
				DrawCube(Vector3{0, 0.62f, 0}, 0.012f, 0.86f, 0.02f, Color{150, 165, 158, 255});    // fuller
				DrawCylinderEx(Vector3{0, 1.05f, 0}, Vector3{0, 1.24f, 0}, 0.030f, 0.0f, 4, steel); // point
				DrawCylinderWiresEx(Vector3{0, 1.05f, 0}, Vector3{0, 1.24f, 0}, 0.030f, 0.0f, 4, steelDk);
			}
			rlPopMatrix();
		}

		// Kick: a boot thrusts up from the bottom-centre of the view and retracts (feedback for the F kick).
		if (kick > 0.0f)
		{
			const float e = 1.0f - kick;       // 0 = start of the kick .. 1 = done
			const float th = std::sin(e * PI); // thrust envelope: 0 -> 1 (foot out) -> 0
			rlPushMatrix();
			rlTranslatef(0.06f, -0.9f + 0.5f * th, -0.55f - 0.4f * th);
			rlRotatef(-62.0f + 48.0f * th, 1, 0, 0);                                    // swing the shin up toward the crosshair
			DrawCube(Vector3{0, 0.2f, 0}, 0.14f, 0.52f, 0.14f, Color{58, 54, 66, 255}); // shin / trouser
			DrawCubeWires(Vector3{0, 0.2f, 0}, 0.14f, 0.52f, 0.14f, Color{34, 31, 40, 255});
			DrawCube(Vector3{0, 0.48f, 0.07f}, 0.16f, 0.14f, 0.28f, Color{46, 36, 30, 255}); // boot
			DrawCubeWires(Vector3{0, 0.48f, 0.07f}, 0.16f, 0.14f, 0.28f, Color{20, 15, 13, 255});
			rlPopMatrix();
		}

		rlEnableDepthTest();
		EndMode3D();
	}
} // namespace adventure
