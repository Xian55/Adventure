#include "raylib.h"
#include "raymath.h"

#include "core/Config.h"
#include "core/Metrics.h"
#include "render/Renderer.h"
#include "render/MetricsOverlay.h"
#include "lua/ScriptEngine.h"

#include <entt/entt.hpp>
#include <cmath>
#include <cstdlib>

using namespace adventure;

namespace
{
	// M0 placeholder component — proves EnTT is wired. Real components arrive in M1/M2.
	struct Spin { float angle; };
}

int main()
{
	SetTraceLogLevel(LOG_WARNING);
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
	InitWindow(config::kWindowW, config::kWindowH, "Adventure - M0 (GRAVEN-style)");
	ChangeDirectory(GetApplicationDirectory());   // resolve scripts/ and assets/ next to the exe

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
	cam.position   = Vector3{ 6.0f, 4.0f, 6.0f };
	cam.target     = Vector3{ 0.0f, 0.5f, 0.0f };
	cam.up         = Vector3{ 0.0f, 1.0f, 0.0f };
	cam.fovy       = 60.0f;
	cam.projection = CAMERA_PERSPECTIVE;

	const Color fog = Color{ 26, 28, 40, 255 };   // dark gothic haze

	// M0 verification hook: ADVENTURE_SHOT=<name> writes a screenshot after a few frames and exits.
	const char* shotPath = getenv("ADVENTURE_SHOT");
	int frame = 0;

	bool showMetrics = true;
	float accumulator = 0.0f;

	while (!WindowShouldClose())
	{
		metrics.beginFrame();

		if (IsKeyPressed(KEY_F3)) showMetrics = !showMetrics;

		// Fixed-step gameplay update.
		{
			Metrics::Scope s(metrics, "update");
			accumulator += GetFrameTime();
			while (accumulator >= config::kFixedDt)
			{
				reg.get<Spin>(spinner).angle += config::kFixedDt * 30.0f;  // deg/s
				accumulator -= config::kFixedDt;
			}
		}

		// Orbit the camera so the pixelated edges + depth are visible.
		float a = reg.get<Spin>(spinner).angle * DEG2RAD;
		cam.position = Vector3{ cosf(a) * 6.0f, 4.0f, sinf(a) * 6.0f };

		{
			Metrics::Scope s(metrics, "scene");
			renderer.beginScene(fog);
				BeginMode3D(cam);
					DrawGrid(20, 1.0f);
					DrawCube(Vector3{ 0, 0.5f, 0 }, 1.5f, 1.0f, 1.5f, MAROON);
					DrawCubeWires(Vector3{ 0, 0.5f, 0 }, 1.5f, 1.0f, 1.5f, Color{ 210, 130, 130, 255 });
					DrawCube(Vector3{ 2.6f, 0.4f, -1.2f }, 0.8f, 0.8f, 0.8f, DARKBLUE);
					DrawCube(Vector3{ -2.4f, 0.3f, 1.4f }, 0.6f, 0.6f, 0.6f, DARKGREEN);
				EndMode3D();
				DrawText("ADVENTURE  M0", 6, 6, 20, RAYWHITE);
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

		if (shotPath && ++frame == 30)
		{
			TakeScreenshot(shotPath);
			break;
		}
	}

	renderer.shutdown();
	sScript->shutdown();
	CloseWindow();
	return 0;
}
