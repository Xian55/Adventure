#include "raylib.h"
#include "raymath.h"

#include "core/Config.h"
#include "core/Metrics.h"
#include "core/ProfileReport.h"
#include "combat/CombatSystem.h"
#include "combat/Destructible.h"
#include "combat/Melee.h"
#include "combat/Rage.h"
#include "items/Inventory.h"
#include "items/Pickup.h"
#include "lua/ScriptEngine.h"
#include "player/JumpMeter.h"
#include "player/PlayerController.h"
#include "render/MetricsOverlay.h"
#include "render/Billboard.h"
#include "render/Prop.h"
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

	EnemyBillboards enemyBillboards;
	enemyBillboards.init();

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
	std::vector<world::Hazard> hazards;
	std::vector<Destructible> props;
	std::vector<Pickup> pickups;
	Inventory inventory;
	PropTuning propTune;
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
		hazards = world::buildHazards(r.data);
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
		// Drop each enemy onto the floor: spawn origins can sit above it, and enemies have no gravity yet.
		for (Enemy& e : enemies)
		{
			const Vector3 he = {e.radius, e.height * 0.5f, e.radius};
			float y = e.position.y + 1.0f;
			const float lo = y - 8.0f;
			while (y > lo && !collision.overlaps(Vector3{e.position.x, y, e.position.z}, he)) // descend to first contact
				y -= 0.05f;
			while (collision.overlaps(Vector3{e.position.x, y, e.position.z}, he)) // lift back out of the floor
				y += 0.02f;
			e.position.y = y;
		}

		// Destructible props: barrels / crates / kegs from prop_* entities (kegs + loot="health" drop a pickup).
		props.clear();
		pickups.clear();
		inventory = Inventory{}; // fresh bag on (re)spawn
		for (const world::Entity& ent : r.data.entities)
		{
			PropKind kind;
			if (ent.classname == "prop_barrel")
				kind = PropKind::Barrel;
			else if (ent.classname == "prop_crate")
				kind = PropKind::Crate;
			else if (ent.classname == "prop_keg")
				kind = PropKind::Keg;
			else
				continue;

			Destructible d;
			d.kind = kind;
			d.radius = kind == PropKind::Crate ? 0.4f : 0.35f;
			d.height = kind == PropKind::Crate ? 0.8f : (kind == PropKind::Keg ? 0.7f : 1.0f);
			d.health = d.maxHealth = kind == PropKind::Crate ? 20.0f : 30.0f;
			const std::string lootKey = ent.str("loot");
			if (lootKey == "health")
				d.dropItem = kItemHealthPotion;
			else if (lootKey == "coin")
				d.dropItem = kItemCoin;
			else if (lootKey == "key")
				d.dropItem = kItemKey;
			else if (kind == PropKind::Keg)
				d.dropItem = kItemCoin; // kegs default to a coin
			d.position = world::mapToEngine(ent.vec3("origin"));
			d.position.y += d.height * 0.5f; // origin at feet -> center
			const Vector3 ph = {d.radius, d.height * 0.5f, d.radius};
			float y = d.position.y + 1.0f;
			const float lo = y - 8.0f;
			while (y > lo && !collision.overlaps(Vector3{d.position.x, y, d.position.z}, ph))
				y -= 0.05f;
			while (collision.overlaps(Vector3{d.position.x, y, d.position.z}, ph))
				y += 0.02f;
			d.position.y = y;
			props.push_back(d);
		}

		TraceLog(LOG_WARNING, "world: %d meshes, %d collision brushes, %d enemies, %d props", (int)geo.meshes.size(), (int)collision.brushCount(), (int)enemies.size(), (int)props.size());
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
	float kickAnim = 0.0f; // boot-kick viewmodel timer (counts down from kickAnimTime)
	const float kickAnimTime = 0.3f;
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
			noclip = !noclip;    // free-fly / level inspection
		if (IsKeyPressed(KEY_B)) // toggle enemy render kind (debug the seam)
			for (Enemy& e : enemies)
				e.render = e.render == RenderKind::Billboard ? RenderKind::Box : RenderKind::Billboard;
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
			{
				const Vector3 kf = {sinf(player.yaw), 0.0f, -cosf(player.yaw)};
				const Vector3 kp = {player.position.x + kf.x * kickReach * 0.7f, player.position.y, player.position.z + kf.z * kickReach * 0.7f};
				damageProps(props, pickups, kp, kickReach * 0.7f, 1000.0f, propTune); // a kick shatters props
			}
			kickCooldown = 0.7f;
			kickAnim = kickAnimTime; // trigger the boot-kick animation
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
					resolveActorProps(player.position, tune.radius, tune.height, props); // props block the player
					jumpMeter.update(player, config::kFixedDt);
				}
				updateMelee(melee, weapon, config::kFixedDt * rageSpeedMul(rage, rageTune));                                                                                                   // berserk swings faster
				const MeleeHitResult hitResult = resolveMeleeHits(melee, weapon, player.position, player.yaw, enemies, enemyTune, rageDamageMul(rage, rageTune), &props, &pickups, &propTune); // hits enemies + smashes props
				addRage(rage, rageTune, hitResult.hits, hitResult.kills);                                                                                                                      // landed melee builds rage
				PlayerTarget tgt;
				tgt.pos = player.position;
				tgt.yaw = player.yaw;
				tgt.shieldRaised = IsMouseButtonDown(MOUSE_BUTTON_RIGHT); // hold RMB to block
				tgt.health = &player.health;
				updateEnemies(enemies, tgt, enemyTune, config::kFixedDt);
				for (Enemy& e : enemies) // props block enemies too (corner them behind a barrel)
					if (e.active && e.state != EnemyState::Dead)
						resolveActorProps(e.position, e.radius, e.height, props);
				updateRage(rage, rageTune, config::kFixedDt); // decay / run the berserk timer
				applyHazards(enemies, hazards, enemyTune, config::kFixedDt);
				updateProps(props, propTune, config::kFixedDt);
				collectPickups(pickups, player.position, inventory, player.health, player.maxHealth, propTune.pickupRadius);
				{
					const Vector3 pf = {player.position.x, player.position.y - tune.height * 0.5f, player.position.z};
					player.health -= world::hazardDamageAt(hazards, pf) * config::kFixedDt; // stand in it, take damage
				}
				if (kickCooldown > 0.0f)
					kickCooldown -= config::kFixedDt;
				if (kickAnim > 0.0f)
					kickAnim -= config::kFixedDt;
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
			for (const world::Hazard& h : hazards) // damage volumes: lava surface + a wire outline
			{
				const Vector3 hc = {(h.min.x + h.max.x) * 0.5f, h.min.y + 0.05f, (h.min.z + h.max.z) * 0.5f};
				DrawPlane(hc, Vector2{h.max.x - h.min.x, h.max.z - h.min.z}, Color{205, 45, 30, 130});
				DrawCubeWires(Vector3{hc.x, (h.min.y + h.max.y) * 0.5f, hc.z}, h.max.x - h.min.x, h.max.y - h.min.y, h.max.z - h.min.z, Color{255, 95, 55, 160});
			}
			for (const Enemy& e : enemies)
			{
				if (!e.active || e.render != RenderKind::Box) // billboards handled below
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
			enemyBillboards.draw(cam, enemies); // RenderKind::Billboard, depth-sorted + state-tinted
			drawProps(props, (float)GetTime());
			drawPickups(pickups, (float)GetTime());
			EndMode3D();
			if (!noclip)
			{
				int vmDir = (melee.phase == MeleePhase::Charge) ? (int)melee.dir : (int)melee.resolved;
				drawViewmodel(bobPhase, weaponBobAmt, (float)GetTime(), (int)melee.phase, phaseProgress(melee, weapon), vmDir, chargeFraction(melee, weapon), kickAnim > 0.0f ? kickAnim / kickAnimTime : 0.0f);
			}
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
		{ // Gameplay HUD (native res, lower-left): health, rage, block/berserk, inventory.
			// Fixed coords: GetRenderHeight() is DPI-unreliable here (see the telemetry note), so a
			// bottom anchor lands off-screen. These sit safely in-frame for the 1280x720 window.
			const int hx = 20, hw = 320, hh = 20, hy = 620;
			const float hpFrac = player.maxHealth > 0.01f ? fmaxf(0.0f, player.health) / player.maxHealth : 0.0f;
			DrawRectangle(hx - 2, hy - 2, hw + 4, hh + 4, Color{20, 15, 15, 220});
			DrawRectangle(hx, hy, (int)(hw * hpFrac), hh, Color{175, 50, 45, 255});
			DrawText(TextFormat("%d / %d", (int)fmaxf(0.0f, player.health), (int)player.maxHealth), hx + hw + 10, hy, 20, RAYWHITE);
			if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
				DrawText("BLOCK", hx + hw + 120, hy, 20, Color{150, 190, 230, 255});
			const int ry = hy - 22;
			const float rf = rageFraction(rage, rageTune);
			DrawRectangle(hx - 2, ry - 2, hw + 4, 14 + 4, Color{20, 15, 15, 220});
			DrawRectangle(hx, ry, (int)(hw * rf), 14, rage.berserk ? Color{255, 170, 40, 255} : Color{210, 120, 40, 255});
			if (rage.berserk)
				DrawText("BERSERK", hx + hw + 10, ry - 3, 20, Color{255, 170, 40, 255});
			DrawText(TextFormat("Coins %d    Keys %d    Potions %d", itemCount(inventory, kItemCoin), itemCount(inventory, kItemKey), itemCount(inventory, kItemHealthPotion)), hx, ry - 30, 20, Color{230, 220, 185, 255});
		}
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
	enemyBillboards.shutdown();
	renderer.shutdown();
	sScript->shutdown();
	CloseWindow();
	return 0;
}
