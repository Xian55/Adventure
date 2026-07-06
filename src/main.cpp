#include "raylib.h"
#include "raymath.h"

#include "core/Config.h"
#include "core/Metrics.h"
#include "core/ProfileReport.h"
#include "render/Renderer.h"
#include "render/MetricsOverlay.h"
#include "render/WorldRenderer.h"
#include "world/BrushGeometry.h"
#include "world/MapParser.h"
#include "lua/ScriptEngine.h"

#include <entt/entt.hpp>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>

using namespace adventure;

namespace
{
	// M0 placeholder component — proves EnTT is wired. Real components arrive in M1/M2.
	struct Spin
	{
		float angle;
	};
} // namespace

int main()
{
	// Headless-ish profiling run: ADVENTURE_PROFILE=<frames> runs the real loop uncapped, records
	// frame times, writes profile.csv, and exits — so performance is measurable without a human.
	const char* profileEnv = getenv("ADVENTURE_PROFILE");
	const int profileFrames = profileEnv ? atoi(profileEnv) : 0;
	const bool profiling = profileFrames > 0;

	SetTraceLogLevel(LOG_WARNING);
	// Uncap the frame rate while profiling so measured times reflect real work, not the vsync cap.
	SetConfigFlags(profiling ? FLAG_WINDOW_HIGHDPI : (FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI));
	InitWindow(config::kWindowW, config::kWindowH, "Adventure - M0 (GRAVEN-style)");
	ChangeDirectory(GetApplicationDirectory()); // resolve scripts/ and assets/ next to the exe

	Renderer renderer;
	renderer.init(config::kWindowW, config::kWindowH, config::kLowW, config::kLowH);

	Metrics& metrics = Metrics::instance();

	// ECS smoke test.
	entt::registry reg;
	entt::entity spinner = reg.create();
	reg.emplace<Spin>(spinner, 0.0f);

	// Lua boot + sandbox proof + load tuning data.
	sScript->init();
	sScript->selfTest();
	sScript->runFile("scripts/tuning.lua");

	Camera3D cam{};
	cam.position = Vector3{6.0f, 4.0f, 6.0f};
	cam.target = Vector3{0.0f, 0.5f, 0.0f};
	cam.up = Vector3{0.0f, 1.0f, 0.0f};
	cam.fovy = 60.0f;
	cam.projection = CAMERA_PERSPECTIVE;

	const Color fog = Color{26, 28, 40, 255}; // dark gothic haze

	WorldRenderer world;
	{
		char* text = LoadFileText("maps/box.map");
		world::MapParseResult mp = world::parseMap(text ? text : "");
		if (text)
			UnloadFileText(text);
		world::WorldGeometry geo = world::buildWorld(mp.data);
		world.load(geo);
		world.setFog(fog, 0.18f);
		TraceLog(LOG_WARNING, "world: %d meshes, %d collision brushes", (int)geo.meshes.size(), (int)geo.collision.size());
	}

	// M0 verification hook: ADVENTURE_SHOT=<name> writes a screenshot after a few frames and exits.
	const char* shotPath = getenv("ADVENTURE_SHOT");
	int frame = 0;

	bool showMetrics = true;
	float accumulator = 0.0f;

	std::vector<float> frameSamples;
	if (profiling)
		frameSamples.reserve((std::size_t)profileFrames);

	while (!WindowShouldClose())
	{
		metrics.beginFrame();

		if (IsKeyPressed(KEY_F3))
			showMetrics = !showMetrics;

		// Fixed-step gameplay update.
		{
			Metrics::Scope s(metrics, "update");
			accumulator += GetFrameTime();
			while (accumulator >= config::kFixedDt)
			{
				reg.get<Spin>(spinner).angle += config::kFixedDt * 30.0f; // deg/s
				accumulator -= config::kFixedDt;
			}
		}

		// Orbit the camera so the pixelated edges + depth are visible.
		float a = reg.get<Spin>(spinner).angle * DEG2RAD;
		cam.position = Vector3{cosf(a) * 6.0f, 4.0f, sinf(a) * 6.0f};

		{
			Metrics::Scope s(metrics, "scene");
			renderer.beginScene(fog);
			BeginMode3D(cam);
			world.draw(cam.position);
			DrawGrid(20, 1.0f);
			EndMode3D();
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
