#include "raylib.h"
#include "raymath.h"

#include "core/Config.h"
#include "core/Metrics.h"
#include "core/ProfileReport.h"
#include "combat/CombatSystem.h"
#include "combat/Melee.h"
#include "combat/Rage.h"
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
	WeaponDef weapon;
	float bobFreq = 9.0f, weaponBob = 0.02f, headBob = 0.035f;
	float kickReach = 1.6f, kickImpulse = 14.0f;
	EnemyTuning enemyTune;
	RageTuning rageTune;
	RageState rage;
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

		sScript->runFile("scripts/weapons/sword.lua");
		weapon.active = (float)sScript->evalNumber("sword.active", weapon.active);
		weapon.recovery = (float)sScript->evalNumber("sword.recovery", weapon.recovery);
		weapon.reach = (float)sScript->evalNumber("sword.reach", weapon.reach);
		weapon.arc = (float)sScript->evalNumber("sword.arc", weapon.arc);
		weapon.damage = (float)sScript->evalNumber("sword.damage", weapon.damage);
		weapon.knockback = (float)sScript->evalNumber("sword.knockback", weapon.knockback);
		weapon.chargeMax = (float)sScript->evalNumber("sword.chargeMax", weapon.chargeMax);
		weapon.chargeDamageMul = (float)sScript->evalNumber("sword.chargeDamageMul", weapon.chargeDamageMul);
		kickReach = (float)sScript->evalNumber("tuning.kickReach", kickReach);
		kickImpulse = (float)sScript->evalNumber("tuning.kickImpulse", kickImpulse);

		enemyTune.blockArc = (float)sScript->evalNumber("tuning.blockArc", enemyTune.blockArc);
		enemyTune.blockReduction = (float)sScript->evalNumber("tuning.blockReduction", enemyTune.blockReduction);
		enemyTune.moveSpeed = (float)sScript->evalNumber("tuning.enemyMoveSpeed", enemyTune.moveSpeed);
		enemyTune.attackRange = (float)sScript->evalNumber("tuning.enemyAttackRange", enemyTune.attackRange);
		enemyTune.attackWindup = (float)sScript->evalNumber("tuning.enemyAttackWindup", enemyTune.attackWindup);
		enemyTune.attackRecover = (float)sScript->evalNumber("tuning.enemyAttackRecover", enemyTune.attackRecover);
		enemyTune.attackReach = (float)sScript->evalNumber("tuning.enemyAttackReach", enemyTune.attackReach);
		enemyTune.attackDamage = (float)sScript->evalNumber("tuning.enemyAttackDamage", enemyTune.attackDamage);
		enemyTune.staggerTime = (float)sScript->evalNumber("tuning.enemyStaggerTime", enemyTune.staggerTime);

		rageTune.max = (float)sScript->evalNumber("tuning.rageMax", rageTune.max);
		rageTune.gainPerHit = (float)sScript->evalNumber("tuning.rageGainPerHit", rageTune.gainPerHit);
		rageTune.gainPerKill = (float)sScript->evalNumber("tuning.rageGainPerKill", rageTune.gainPerKill);
		rageTune.decayPerSec = (float)sScript->evalNumber("tuning.rageDecayPerSec", rageTune.decayPerSec);
		rageTune.decayDelay = (float)sScript->evalNumber("tuning.rageDecayDelay", rageTune.decayDelay);
		rageTune.berserkDuration = (float)sScript->evalNumber("tuning.berserkDuration", rageTune.berserkDuration);
		rageTune.damageMul = (float)sScript->evalNumber("tuning.berserkDamageMul", rageTune.damageMul);
		rageTune.speedMul = (float)sScript->evalNumber("tuning.berserkSpeedMul", rageTune.speedMul);
	};

	// --- Map (hot-reloadable: F6) ---
	const char* mapEnv = getenv("ADVENTURE_MAP");
	const char* mapPath = mapEnv ? mapEnv : "maps/training.map";
	WorldRenderer world;
	world::CollisionWorld collision;
	world::WorldGeometry geo;
	Player player;
	std::vector<Enemy> enemies;
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
		player.health = player.maxHealth; // fresh spawn / respawn
		rage = RageState{};               // lose the meter on death

		// Spawn skeletons from monster_skeleton entities; if the map has none, drop a few in front so the
		// combat slice always has targets.
		enemies.clear();
		for (const world::Entity& ent : r.data.entities)
			if (ent.classname == "monster_skeleton")
			{
				Enemy e;
				e.position = world::mapToEngine(ent.vec3("origin"));
				e.position.y += e.height * 0.5f;
				enemies.push_back(e);
			}
		if (enemies.empty())
		{
			const float floorY = player.position.y - tune.height * 0.5f;
			for (int i = 0; i < 3; ++i)
			{
				Enemy e;
				e.position = {player.position.x + (i - 1) * 2.0f, floorY + e.height * 0.5f, player.position.z - 5.0f};
				enemies.push_back(e);
			}
		}
		TraceLog(LOG_WARNING, "world: %d meshes, %d collision brushes, %d enemies", (int)geo.meshes.size(), (int)collision.brushCount(), (int)enemies.size());
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
	MeleeState melee;
	float kickCooldown = 0.0f;
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
		// Attack: hold LMB to wind up, the held movement key picks the direction, release to strike.
		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
			beginCharge(melee);
		if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
		{
			SwingDir d = SwingDir::Neutral;
			if (IsKeyDown(KEY_A))
				d = SwingDir::Left;
			else if (IsKeyDown(KEY_D))
				d = SwingDir::Right;
			else if (IsKeyDown(KEY_W))
				d = SwingDir::Forward;
			else if (IsKeyDown(KEY_S))
				d = SwingDir::Overhead;
			setSwingDir(melee, d);
		}
		if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
			releaseSwing(melee);
		if (IsKeyPressed(KEY_F) && kickCooldown <= 0.0f) // kick: knock enemies back
		{
			tryKick(player.position, player.yaw, enemies, kickReach, kickImpulse, enemyTune);
			kickCooldown = 0.7f;
		}
		if (shotPath) // auto charge+release so the screenshot catches a mid-swing pose
		{
			if (melee.phase == MeleePhase::Idle)
				beginCharge(melee);
			else if (melee.phase == MeleePhase::Charge && melee.chargeTime > 0.12f)
				releaseSwing(melee);
		}

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
				updateMelee(melee, weapon, config::kFixedDt * rageSpeedMul(rage, rageTune)); // berserk swings faster
				const MeleeHitResult hitResult = resolveMeleeHits(melee, weapon, player.position, player.yaw, enemies, enemyTune, rageDamageMul(rage, rageTune));
				addRage(rage, rageTune, hitResult.hits, hitResult.kills); // landed melee builds rage
				PlayerTarget tgt;
				tgt.pos = player.position;
				tgt.yaw = player.yaw;
				tgt.shieldRaised = IsMouseButtonDown(MOUSE_BUTTON_RIGHT); // hold RMB to block
				tgt.health = &player.health;
				updateEnemies(enemies, tgt, enemyTune, config::kFixedDt);
				updateRage(rage, rageTune, config::kFixedDt); // decay / run the berserk timer
				if (kickCooldown > 0.0f)
					kickCooldown -= config::kFixedDt;
				accumulator -= config::kFixedDt;
				if (player.health <= 0.0f) // died -> respawn (resets map, enemies, health)
				{
					loadMap(mapPath);
					accumulator = 0.0f;
					break;
				}
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
			for (const Enemy& e : enemies)
			{
				if (!e.active)
					continue;
				Color c = e.state == EnemyState::Dead      ? Color{95, 75, 72, 255}
				          : e.state == EnemyState::Stagger ? Color{225, 120, 110, 255}
				          : e.state == EnemyState::Windup  ? Color{245, 205, 90, 255} // telegraph: about to strike
				                                           : Color{205, 200, 185, 255};
				float bh = e.state == EnemyState::Dead ? e.height * 0.35f : e.height;
				Vector3 box = {e.position.x, e.position.y - (e.height - bh) * 0.5f, e.position.z};
				DrawCube(box, e.radius * 2.0f, bh, e.radius * 2.0f, c);
				DrawCubeWires(box, e.radius * 2.0f, bh, e.radius * 2.0f, Color{40, 40, 45, 255});
			}
			EndMode3D();
			if (!noclip)
			{
				int vmDir = (melee.phase == MeleePhase::Charge) ? (int)melee.dir : (int)melee.resolved;
				drawViewmodel(bobPhase, weaponBobAmt, (float)GetTime(), (int)melee.phase, phaseProgress(melee, weapon), vmDir, chargeFraction(melee, weapon));
			}
			DrawText("ADVENTURE  M1", 6, 6, 20, RAYWHITE);
			// Health bar (bottom-left of the low-res frame) + block indicator.
			{
				const int rh = renderer.lowH();
				const float hp = player.maxHealth > 0.01f ? fmaxf(0.0f, player.health) / player.maxHealth : 0.0f;
				const int bx = 8, bw = 120, bh2 = 8, by = rh - 16;
				DrawRectangle(bx - 1, by - 1, bw + 2, bh2 + 2, Color{20, 15, 15, 220});
				DrawRectangle(bx, by, (int)(bw * hp), bh2, Color{170, 50, 45, 255});
				if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
					DrawText("BLOCK", bx + bw + 6, by, 10, Color{150, 190, 230, 255});
				// Rage meter above the health bar; glows + flashes "BERSERK" when maxed.
				const float rf = rageFraction(rage, rageTune);
				const int ry = by - 9;
				DrawRectangle(bx - 1, ry - 1, bw + 2, 7, Color{20, 15, 15, 220});
				const Color rageCol = rage.berserk ? Color{255, (unsigned char)(150 + (int)(80 * sinf((float)GetTime() * 18.0f))), 30, 255} : Color{210, 120, 40, 255};
				DrawRectangle(bx, ry, (int)(bw * rf), 5, rageCol);
				if (rage.berserk)
					DrawText("BERSERK", bx + bw + 6, ry - 2, 10, Color{255, 170, 40, 255});
			}
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
