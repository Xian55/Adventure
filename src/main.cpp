#include "raylib.h"
#include "raymath.h"

#include "core/Config.h"
#include "core/Metrics.h"
#include "core/ProfileReport.h"
#include "lua/ScriptEngine.h"
#include "player/JumpMeter.h"
#include "player/PlayerController.h"
#include "render/MetricsOverlay.h"
#include "render/Renderer.h"
#include "render/Viewmodel.h"
#include "render/WorldRenderer.h"
#include "world/BrushGeometry.h"
#include "world/CollisionWorld.h"
#include "world/MapParser.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>

using namespace adventure;

int main()
{
	// ADVENTURE_PROFILE=<frames>: run the real loop uncapped, write profile.csv, exit.
	const char* profileEnv = getenv("ADVENTURE_PROFILE");
	const int profileFrames = profileEnv ? atoi(profileEnv) : 0;
	const bool profiling = profileFrames > 0;

	SetTraceLogLevel(LOG_WARNING);
	SetConfigFlags(profiling ? FLAG_WINDOW_HIGHDPI : (FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI));
	InitWindow(config::kWindowW, config::kWindowH, "Adventure - M1 (GRAVEN-style)");
	ChangeDirectory(GetApplicationDirectory());
	DisableCursor(); // capture mouse for look

	Renderer renderer;
	renderer.init(config::kWindowW, config::kWindowH, config::kLowW, config::kLowH);

	Metrics& metrics = Metrics::instance();
	const Color fog = Color{26, 28, 40, 255};

	sScript->init();
	sScript->selfTest();

	// --- Tuning (hot-reloadable: F5) ---
	MoveTuning tune;
	float bobFreq = 9.0f, weaponBob = 0.02f, headBob = 0.035f;
	auto loadTuning = [&]() {
		sScript->runFile("scripts/tuning.lua");
		tune.moveSpeed = (float)sScript->evalNumber("tuning.moveSpeed", tune.moveSpeed);
		tune.sprintSpeed = (float)sScript->evalNumber("tuning.sprintSpeed", tune.sprintSpeed);
		tune.accel = (float)sScript->evalNumber("tuning.accel", tune.accel);
		tune.airAccel = (float)sScript->evalNumber("tuning.airAccel", tune.airAccel);
		tune.friction = (float)sScript->evalNumber("tuning.friction", tune.friction);
		tune.stopSpeed = (float)sScript->evalNumber("tuning.stopSpeed", tune.stopSpeed);
		tune.gravity = (float)sScript->evalNumber("tuning.gravity", tune.gravity);
		tune.jumpSpeed = (float)sScript->evalNumber("tuning.jumpSpeed", tune.jumpSpeed);
		tune.stepHeight = (float)sScript->evalNumber("tuning.stepHeight", tune.stepHeight);
		bobFreq = (float)sScript->evalNumber("tuning.bobFreq", bobFreq);
		weaponBob = (float)sScript->evalNumber("tuning.weaponBob", weaponBob);
		headBob = (float)sScript->evalNumber("tuning.headBob", headBob);
	};

	// --- Map (hot-reloadable: F6) ---
	const char* mapEnv = getenv("ADVENTURE_MAP");
	const char* mapPath = mapEnv ? mapEnv : "maps/training.map";
	WorldRenderer world;
	world::CollisionWorld collision;
	world::WorldGeometry geo;
	Player player;
	auto loadMap = [&](const char* path) {
		char* txt = LoadFileText(path);
		world::MapParseResult r = world::parseMap(txt ? txt : "");
		if (txt)
			UnloadFileText(txt);
		geo = world::buildWorld(r.data);
		world.unload();
		world.load(geo);
		world.setFog(fog, 0.10f);
		collision.build(geo);
		if (const world::Entity* sp = r.data.first("info_player_start"))
		{
			player.position = world::mapToEngine(sp->vec3("origin"));
			player.position.y += tune.height * 0.5f; // origin at feet -> AABB center
		}
		player.velocity = Vector3{0, 0, 0};
		TraceLog(LOG_WARNING, "world: %d meshes, %d collision brushes", (int)geo.meshes.size(), (int)collision.brushCount());
	};

	loadTuning();
	loadMap(mapPath);

	Camera3D cam{};
	cam.up = Vector3{0.0f, 1.0f, 0.0f};
	cam.fovy = 70.0f;
	cam.projection = CAMERA_PERSPECTIVE;

	const char* shotPath = getenv("ADVENTURE_SHOT");
	int frame = 0;

	bool showMetrics = true;
	bool showTelemetry = true;
	bool noclip = false;
	JumpMeter jumpMeter;
	float accumulator = 0.0f;
	float bobPhase = 0.0f;

	std::vector<float> frameSamples;
	if (profiling)
		frameSamples.reserve((std::size_t)profileFrames);

	while (!WindowShouldClose())
	{
		metrics.beginFrame();

		if (IsKeyPressed(KEY_F3))
			showMetrics = !showMetrics;
		if (IsKeyPressed(KEY_F4))
			showTelemetry = !showTelemetry;
		if (IsKeyPressed(KEY_F5))
			loadTuning(); // hot-reload feel
		if (IsKeyPressed(KEY_F6))
			loadMap(mapPath); // hot-reload level
		if (IsKeyPressed(KEY_V))
			noclip = !noclip; // free-fly / level inspection

		// Mouse look (per display frame for smoothness).
		{
			const float sens = 0.0022f;
			Vector2 md = GetMouseDelta();
			player.yaw += md.x * sens;
			player.pitch -= md.y * sens;
			const float lim = 1.55f;
			if (player.pitch > lim)
				player.pitch = lim;
			if (player.pitch < -lim)
				player.pitch = -lim;
		}

		// Fixed-step movement (walk/collide) or free-fly (noclip).
		{
			Metrics::Scope s(metrics, "update");
			MoveInput in;
			in.forward = (IsKeyDown(KEY_W) ? 1.0f : 0.0f) - (IsKeyDown(KEY_S) ? 1.0f : 0.0f);
			in.right = (IsKeyDown(KEY_D) ? 1.0f : 0.0f) - (IsKeyDown(KEY_A) ? 1.0f : 0.0f);
			in.jump = IsKeyDown(KEY_SPACE);
			in.crouch = IsKeyDown(KEY_LEFT_CONTROL);
			in.sprint = IsKeyDown(KEY_LEFT_SHIFT);
			accumulator += GetFrameTime();
			while (accumulator >= config::kFixedDt)
			{
				if (noclip)
				{
					Vector3 f = {sinf(player.yaw) * cosf(player.pitch), sinf(player.pitch), -cosf(player.yaw) * cosf(player.pitch)};
					Vector3 rt = {cosf(player.yaw), 0.0f, sinf(player.yaw)};
					float sp = tune.sprintSpeed * (in.sprint ? 3.0f : 1.5f) * config::kFixedDt;
					float up = (in.jump ? 1.0f : 0.0f) - (in.crouch ? 1.0f : 0.0f);
					player.position.x += (f.x * in.forward + rt.x * in.right) * sp;
					player.position.y += (f.y * in.forward + up) * sp;
					player.position.z += (f.z * in.forward + rt.z * in.right) * sp;
					player.velocity = Vector3{0, 0, 0};
					player.onGround = false;
				}
				else
				{
					updatePlayer(player, in, collision, tune, config::kFixedDt);
					jumpMeter.update(player, config::kFixedDt);
				}
				accumulator -= config::kFixedDt;
			}
		}

		// Bob (from horizontal speed) + first-person camera from the player.
		float weaponBobAmt = 0.0f;
		{
			float hs = sqrtf(player.velocity.x * player.velocity.x + player.velocity.z * player.velocity.z);
			bobPhase += hs * GetFrameTime() * bobFreq * 0.1f;
			float moveFrac = tune.sprintSpeed > 0.01f ? fminf(1.0f, hs / tune.sprintSpeed) : 0.0f;
			weaponBobAmt = weaponBob * moveFrac;

			Vector3 eye = player.position;
			eye.y += tune.eyeHeight - tune.height * 0.5f + sinf(bobPhase * 2.0f) * headBob * moveFrac;
			Vector3 dir = {sinf(player.yaw) * cosf(player.pitch), sinf(player.pitch), -cosf(player.yaw) * cosf(player.pitch)};
			cam.position = eye;
			cam.target = Vector3{eye.x + dir.x, eye.y + dir.y, eye.z + dir.z};
		}

		{
			Metrics::Scope s(metrics, "scene");
			renderer.beginScene(fog);
			BeginMode3D(cam);
			world.draw(cam.position);
			EndMode3D();
			if (!noclip)
				drawViewmodel(bobPhase, weaponBobAmt, (float)GetTime());
			DrawText("ADVENTURE  M1", 6, 6, 20, RAYWHITE);
			renderer.endScene();
		}

		{
			Metrics::Scope s(metrics, "post");
			renderer.postProcess();
		}

		metrics.setLuaBytes(sScript->luaMemoryBytes());

		BeginDrawing();
		ClearBackground(BLACK);
		{
			Metrics::Scope s(metrics, "present");
			renderer.blit();
		}
		drawMetricsOverlay(metrics, showMetrics);
		if (showTelemetry)
		{
			int ty = 156; // below the metrics panel (fixed coords; GetRenderHeight is DPI-unreliable)
			const Color tc = Color{170, 215, 255, 255};
			DrawText(TextFormat("TRAINING  V noclip%s  F5 tuning  F6 map   speed %.2f  %s",
			                    noclip ? " [ON]" : "",
			                    JumpMeter::horizontalSpeed(player),
			                    noclip ? "fly" : (player.onGround ? "ground" : "air")),
			         12,
			         ty,
			         20,
			         tc);
			DrawText(TextFormat("dims: stand %.2f  crouch %.2f  radius %.2f  eye %.2f", tune.height, tune.crouchHeight, tune.radius, tune.eyeHeight),
			         12,
			         ty + 26,
			         20,
			         tc);
			DrawText(TextFormat("jump: dist %.2f  height %.2f  air %.2fs", jumpMeter.last().distance, jumpMeter.last().height, jumpMeter.last().airtime),
			         12,
			         ty + 52,
			         20,
			         tc);
			DrawText(TextFormat("max:  dist %.2f  height %.2f  pos %.1f %.1f %.1f", jumpMeter.maxDistance(), jumpMeter.maxHeight(), player.position.x, player.position.y, player.position.z),
			         12,
			         ty + 78,
			         20,
			         tc);
		}
		EndDrawing();

		metrics.endFrame();

		if (profiling)
		{
			frameSamples.push_back(metrics.frameMsRaw());
			if ((int)frameSamples.size() >= profileFrames)
			{
				const bool ok = writeProfileCsv("profile.csv", metrics, frameSamples);
				const FrameStats fs = computeFrameStats(frameSamples);
				std::printf("[adventure] profile: %zu frames  avg %.2f ms  p95 %.2f ms  max %.2f ms  (%s)\n",
				            fs.count,
				            fs.avgMs,
				            fs.p95Ms,
				            fs.maxMs,
				            ok ? "profile.csv written" : "CSV write FAILED");
				std::fflush(stdout);
				break;
			}
		}

		if (shotPath && ++frame == 30)
		{
			TakeScreenshot(shotPath);
			break;
		}
	}

	world.unload();
	renderer.shutdown();
	sScript->shutdown();
	CloseWindow();
	return 0;
}
