#include "raylib.h"
#include "raymath.h"

#include "core/Config.h"
#include "core/Metrics.h"
#include "core/ProfileReport.h"
#include "combat/CombatSystem.h"
#include "combat/Destructible.h"
#include "combat/Loadout.h"
#include "combat/Melee.h"
#include "combat/Rage.h"
#include "input/InputMap.h"
#include "input/InputQuery.h"
#include "items/Collide.h"
#include "items/Container.h"
#include "items/Inventory.h"
#include "items/Pickup.h"
#include "mech/Mechanisms.h"
#include "lua/ScriptEngine.h"
#include "player/JumpMeter.h"
#include "player/PlayerController.h"
#include "rpg/SkillTree.h"
#include "render/MetricsOverlay.h"
#include "render/Billboard.h"
#include "render/Mech.h"
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

	// Keybindings: load keybindings.cfg (next to the exe), then rewrite it so new actions appear while the
	// player's overrides are kept — the file is the rebind interface.
	InputMap keys = defaultBindings();
	{
		const char* kCfg = "keybindings.cfg";
		if (FileExists(kCfg))
		{
			char* t = LoadFileText(kCfg);
			keys = loadBindings(t ? t : "");
			if (t)
				UnloadFileText(t);
		}
		std::string txt = saveBindings(keys);
		SaveFileText(kCfg, txt.data());
	}

	sScript->init();
	sScript->selfTest();

	// --- Tuning (hot-reloadable: F5) ---
	MoveTuning tune;
	WeaponDef weapon;   // the active weapon's def (equipped)
	WeaponDef swordDef; // the Lua-tuned sword base; other weapons derive from it
	int equippedItem = kItemSword;
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
		swordDef.active = (float)sScript->evalNumber("sword.active", swordDef.active);
		swordDef.recovery = (float)sScript->evalNumber("sword.recovery", swordDef.recovery);
		swordDef.reach = (float)sScript->evalNumber("sword.reach", swordDef.reach);
		swordDef.arc = (float)sScript->evalNumber("sword.arc", swordDef.arc);
		swordDef.damage = (float)sScript->evalNumber("sword.damage", swordDef.damage);
		swordDef.knockback = (float)sScript->evalNumber("sword.knockback", swordDef.knockback);
		swordDef.chargeMax = (float)sScript->evalNumber("sword.chargeMax", swordDef.chargeMax);
		swordDef.chargeDamageMul = (float)sScript->evalNumber("sword.chargeDamageMul", swordDef.chargeDamageMul);
		weapon = weaponDefFor(equippedItem, swordDef); // re-derive the active weapon after a tuning reload
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
	std::vector<Container> containers;
	std::vector<Door> doors;
	std::vector<Lever> levers;
	std::vector<Plate> plates;
	Inventory inventory;
	PropTuning propTune;
	SkillState skills; // persists across respawn (not reset in loadMap)
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
		player.maxHealth = 100.0f + deriveStats(skills).maxHealthBonus; // Toughness raises the cap
		player.health = player.maxHealth;                               // fresh spawn / respawn
		rage = RageState{};                                             // lose the meter on death

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
		containers.clear();
		doors.clear();
		levers.clear();
		plates.clear();
		inventory = Inventory{};           // fresh bag on (re)spawn
		addItem(inventory, kItemSword, 1); // start with all three so weapon-swap (Q) works immediately
		addItem(inventory, kItemDagger, 1);
		addItem(inventory, kItemMace, 1);
		equippedItem = kItemSword;
		weapon = weaponDefFor(equippedItem, swordDef);
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
			else if (lootKey == "dagger")
				d.dropItem = kItemDagger;
			else if (lootKey == "mace")
				d.dropItem = kItemMace;
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

		// Chests: prop_chest entities. Contents from coins/potions/keys count keys; lock>0 = locked.
		for (const world::Entity& ent : r.data.entities)
		{
			if (ent.classname != "prop_chest")
				continue;
			Container c;
			c.position = world::mapToEngine(ent.vec3("origin"));
			c.position.y += c.height * 0.5f;
			c.lockId = (int)ent.number("lock", 0.0f);
			c.locked = c.lockId > 0;
			const int coins = (int)ent.number("coins", 0.0f);
			const int potions = (int)ent.number("potions", 0.0f);
			const int keys = (int)ent.number("keys", 0.0f);
			if (coins > 0)
				c.contents.push_back(ItemStack{kItemCoin, coins});
			if (potions > 0)
				c.contents.push_back(ItemStack{kItemHealthPotion, potions});
			if (keys > 0)
				c.contents.push_back(ItemStack{kItemKey, keys});
			const std::string wpn = ent.str("weapon");
			if (wpn == "dagger")
				c.contents.push_back(ItemStack{kItemDagger, 1});
			else if (wpn == "mace")
				c.contents.push_back(ItemStack{kItemMace, 1});
			const Vector3 ch = {c.radius, c.height * 0.5f, c.radius};
			float y = c.position.y + 1.0f;
			const float lo = y - 8.0f;
			while (y > lo && !collision.overlaps(Vector3{c.position.x, y, c.position.z}, ch))
				y -= 0.05f;
			while (collision.overlaps(Vector3{c.position.x, y, c.position.z}, ch))
				y += 0.02f;
			c.position.y = y;
			containers.push_back(c);
		}

		// Mechanisms: doors (func_door), levers, pressure plates. Wired by target/targetname.
		const float ms = config::kMapScale;
		for (const world::Entity& ent : r.data.entities)
		{
			if (ent.classname == "func_door")
			{
				Door d;
				d.targetname = ent.str("targetname");
				d.lockId = (int)ent.number("lock", 0.0f);
				d.locked = d.lockId > 0;
				const Vector3 size = ent.vec3("size", Vector3{64, 12, 96}); // map dims: width(x) depth(y) height(z)
				d.half = {size.x * ms * 0.5f, size.z * ms * 0.5f, size.y * ms * 0.5f};
				const Vector3 mv = ent.vec3("move", Vector3{0, 0, size.z}); // map translation when open (default: up its height)
				d.openMove = {mv.x * ms, mv.z * ms, -mv.y * ms};
				d.position = world::mapToEngine(ent.vec3("origin"));
				d.position.y += d.half.y; // origin at feet -> center
				doors.push_back(d);
			}
			else if (ent.classname == "lever")
			{
				Lever l;
				l.position = world::mapToEngine(ent.vec3("origin"));
				l.target = ent.str("target");
				levers.push_back(l);
			}
			else if (ent.classname == "func_plate" || ent.classname == "trigger_plate")
			{
				Plate p;
				p.position = world::mapToEngine(ent.vec3("origin"));
				const Vector3 size = ent.vec3("size", Vector3{64, 8, 64});
				p.half = {size.x * ms * 0.5f, size.z * ms * 0.5f, size.y * ms * 0.5f};
				p.position.y += p.half.y; // sit the pad on the floor
				p.target = ent.str("target");
				plates.push_back(p);
			}
		}

		TraceLog(LOG_WARNING, "world: %d meshes, %d brushes, %d enemies, %d props, %d chests, %d doors", (int)geo.meshes.size(), (int)collision.brushCount(), (int)enemies.size(), (int)props.size(), (int)containers.size(), (int)doors.size());
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
	bool showSkills = false; // skill-tree overlay open (pauses the sim)
	int skillSel = 0;

	std::vector<float> frameSamples;
	if (profiling)
		frameSamples.reserve((std::size_t)profileFrames);

	while (!WindowShouldClose())
	{
		metrics.beginFrame();

		// Skill-tree overlay (pauses the sim). Nav: Up/Down select, Enter to unlock.
		if (actionPressed(keys, Action::SkillMenu))
		{
			showSkills = !showSkills;
			if (showSkills)
				EnableCursor();
			else
				DisableCursor();
		}
		if (showSkills)
		{
			if (IsKeyPressed(KEY_DOWN))
				skillSel = (skillSel + 1) % SKILL_COUNT;
			if (IsKeyPressed(KEY_UP))
				skillSel = (skillSel + SKILL_COUNT - 1) % SKILL_COUNT;
			if (IsKeyPressed(KEY_ENTER) && unlockSkill(skills, skillSel))
				player.maxHealth = 100.0f + deriveStats(skills).maxHealthBonus; // Toughness may have raised the cap
		}

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
		// Attack: hold to wind up, the held movement key picks the direction, release to strike (rebindable).
		if (actionPressed(keys, Action::Attack))
			beginCharge(melee);
		if (actionDown(keys, Action::Attack))
		{
			SwingDir d = SwingDir::Neutral;
			if (actionDown(keys, Action::MoveLeft))
				d = SwingDir::Left;
			else if (actionDown(keys, Action::MoveRight))
				d = SwingDir::Right;
			else if (actionDown(keys, Action::MoveForward))
				d = SwingDir::Forward;
			else if (actionDown(keys, Action::MoveBack))
				d = SwingDir::Overhead;
			setSwingDir(melee, d);
		}
		if (actionReleased(keys, Action::Attack))
			releaseSwing(melee);
		if (actionPressed(keys, Action::Kick) && kickCooldown <= 0.0f) // kick: knock enemies back
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
		if (actionPressed(keys, Action::NextWeapon)) // cycle the weapons you hold
		{
			const int n = nextWeapon(inventory, equippedItem);
			if (n != kItemNone && n != equippedItem)
			{
				equippedItem = n;
				weapon = weaponDefFor(equippedItem, swordDef);
			}
		}
		// Use (E): the nearest facing lever, then door, then chest.
		const int leverTarget = nearestLever(levers, player.position, player.yaw, 2.2f);
		const int doorTarget = nearestDoor(doors, player.position, player.yaw, 2.2f);
		const int chestTarget = nearestContainer(containers, player.position, player.yaw, 2.2f);
		if (actionPressed(keys, Action::Interact) && !showSkills)
		{
			const bool lockpick = deriveStats(skills).lockpick; // Lockpicking opens locks without a key
			if (leverTarget >= 0)
				levers[leverTarget].on = !levers[leverTarget].on;
			else if (doorTarget >= 0)
				useDoor(doors[doorTarget], inventory, lockpick);
			else if (chestTarget >= 0)
				tryOpenContainer(containers[chestTarget], inventory, pickups, lockpick);
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
			const float sens = showSkills ? 0.0f : 0.0022f; // freeze the look while the skill menu is open
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
			const Stats st = deriveStats(skills); // skill effects
			MoveTuning mt = tune;                 // move-speed skill applies to a working copy
			mt.moveSpeed *= st.moveSpeedMul;
			mt.sprintSpeed *= st.moveSpeedMul;
			RageTuning rt = rageTune; // rage-build skill scales gains
			rt.gainPerHit *= st.rageBuildMul;
			rt.gainPerKill *= st.rageBuildMul;
			MoveInput in;
			in.forward = (actionDown(keys, Action::MoveForward) ? 1.0f : 0.0f) - (actionDown(keys, Action::MoveBack) ? 1.0f : 0.0f);
			in.right = (actionDown(keys, Action::MoveRight) ? 1.0f : 0.0f) - (actionDown(keys, Action::MoveLeft) ? 1.0f : 0.0f);
			in.jump = actionDown(keys, Action::Jump);
			in.crouch = actionDown(keys, Action::Crouch);
			in.sprint = actionDown(keys, Action::Sprint);
			if (!showSkills)
				accumulator += GetFrameTime(); // paused while the skill menu is open
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
					updatePlayer(player, in, collision, mt, config::kFixedDt);
					{ // props + chests are solid boxes: block from the sides, stand on top (jump onto them)
						std::vector<SolidBox> solids;
						for (const Destructible& p : props)
							if (p.active && !p.broken)
								solids.push_back(SolidBox{p.position, {p.radius, p.height * 0.5f, p.radius}});
						for (const Container& c : containers)
							solids.push_back(SolidBox{c.position, {c.radius, c.height * 0.5f, c.radius}});
						for (const Door& dr : doors)
							solids.push_back(doorSolid(dr));
						collideActorBoxes(player.position, player.velocity, player.onGround, Vector3{tune.radius, tune.height * 0.5f, tune.radius}, solids);
					}
					jumpMeter.update(player, config::kFixedDt);
				}
				updateMelee(melee, weapon, config::kFixedDt * rageSpeedMul(rage, rageTune));                                                                                                                  // berserk swings faster
				const MeleeHitResult hitResult = resolveMeleeHits(melee, weapon, player.position, player.yaw, enemies, enemyTune, rageDamageMul(rage, rageTune) * st.damageMul, &props, &pickups, &propTune); // Power skill scales damage
				addRage(rage, rt, hitResult.hits, hitResult.kills);                                                                                                                                           // landed melee builds rage (Adrenaline scales)
				PlayerTarget tgt;
				tgt.pos = player.position;
				tgt.yaw = player.yaw;
				tgt.shieldRaised = actionDown(keys, Action::Block); // hold to block (rebindable)
				tgt.health = &player.health;
				updateEnemies(enemies, tgt, enemyTune, config::kFixedDt);
				for (Enemy& e : enemies) // a kill grants a skill point (placeholder income until quests/secrets)
					if (e.state == EnemyState::Dead && !e.scored)
					{
						skills.points++;
						e.scored = true;
					}
				for (Enemy& e : enemies) // props block enemies too (corner them behind a barrel)
					if (e.active && e.state != EnemyState::Dead)
					{
						resolveActorProps(e.position, e.radius, e.height, props);
						resolveActorContainers(e.position, e.radius, e.height, containers);
					}
				updateRage(rage, rageTune, config::kFixedDt); // decay / run the berserk timer
				applyHazards(enemies, hazards, enemyTune, config::kFixedDt);
				updateProps(props, propTune, config::kFixedDt);
				collectPickups(pickups, player.position, inventory, player.health, player.maxHealth, propTune.pickupRadius);
				{ // mechanisms: plate weight (actors/props) -> activate linked doors -> animate
					std::vector<Vector3> occupants;
					occupants.push_back(Vector3{player.position.x, player.position.y - tune.height * 0.5f, player.position.z});
					for (const Enemy& en : enemies)
						if (en.active)
							occupants.push_back(Vector3{en.position.x, en.position.y - en.height * 0.5f, en.position.z});
					for (const Destructible& pr : props)
						if (pr.active && !pr.broken)
							occupants.push_back(Vector3{pr.position.x, pr.position.y - pr.height * 0.5f, pr.position.z});
					updatePlates(plates, occupants);
					applyActivations(doors, levers, plates);
					updateDoors(doors, config::kFixedDt);
				}
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
			drawContainers(containers);
			drawDoors(doors);
			drawLevers(levers);
			drawPlates(plates);
			drawProps(props, (float)GetTime());
			drawPickups(pickups, (float)GetTime());
			EndMode3D();
			if (!noclip)
			{
				int vmDir = (melee.phase == MeleePhase::Charge) ? (int)melee.dir : (int)melee.resolved;
				const int weaponModel = equippedItem == kItemDagger ? 1 : (equippedItem == kItemMace ? 2 : 0);
				drawViewmodel(bobPhase, weaponBobAmt, (float)GetTime(), (int)melee.phase, phaseProgress(melee, weapon), vmDir, chargeFraction(melee, weapon), kickAnim > 0.0f ? kickAnim / kickAnimTime : 0.0f, weaponModel);
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
		if (showSkills)
		{ // Skill-tree overlay (native res)
			DrawRectangle(0, 0, GetRenderWidth(), GetRenderHeight(), Color{0, 0, 0, 150});
			const int px = 90, py = 120;
			DrawText("SKILL TREE", px, py - 46, 34, Color{240, 230, 190, 255});
			DrawText(TextFormat("Points: %d     Up/Down select   Enter unlock   %s close", skills.points, codeName(actionCode(keys, Action::SkillMenu)).c_str()), px, py - 8, 20, Color{200, 210, 220, 255});
			for (int i = 0; i < SKILL_COUNT; ++i)
			{
				const SkillNode& n = skillNode(i);
				const int rank = skills.rank[i];
				const int cost = skillCost(i, rank);
				const bool sel = (i == skillSel);
				const char* status = rank >= n.maxRank ? "MAX" : (canUnlock(skills, i) ? "available" : "locked");
				const int y = py + 34 + i * 34;
				if (sel)
					DrawRectangle(px - 10, y - 5, 760, 32, Color{60, 55, 40, 200});
				DrawText(TextFormat("%-12s [%s]  rank %d/%d  cost %d  - %s", n.name, n.tree, rank, n.maxRank, cost < 0 ? 0 : cost, status), px, y, 22, sel ? Color{255, 240, 180, 255} : Color{210, 210, 215, 255});
			}
			const Stats sst = deriveStats(skills);
			DrawText(TextFormat("HP +%d    dmg x%.2f    speed x%.2f    rage x%.2f    lockpick %s", (int)sst.maxHealthBonus, sst.damageMul, sst.moveSpeedMul, sst.rageBuildMul, sst.lockpick ? "yes" : "no"), px, py + 34 + SKILL_COUNT * 34 + 14, 20, Color{150, 205, 150, 255});
		}
		{ // Gameplay HUD (native res, lower-left): health, rage, block/berserk, inventory.
			// Fixed coords: GetRenderHeight() is DPI-unreliable here (see the telemetry note), so a
			// bottom anchor lands off-screen. These sit safely in-frame for the 1280x720 window.
			const int hx = 20, hw = 320, hh = 20, hy = 620;
			const float hpFrac = player.maxHealth > 0.01f ? fmaxf(0.0f, player.health) / player.maxHealth : 0.0f;
			DrawRectangle(hx - 2, hy - 2, hw + 4, hh + 4, Color{20, 15, 15, 220});
			DrawRectangle(hx, hy, (int)(hw * hpFrac), hh, Color{175, 50, 45, 255});
			DrawText(TextFormat("%d / %d", (int)fmaxf(0.0f, player.health), (int)player.maxHealth), hx + hw + 10, hy, 20, RAYWHITE);
			if (actionDown(keys, Action::Block))
				DrawText("BLOCK", hx + hw + 120, hy, 20, Color{150, 190, 230, 255});
			const int ry = hy - 22;
			const float rf = rageFraction(rage, rageTune);
			DrawRectangle(hx - 2, ry - 2, hw + 4, 14 + 4, Color{20, 15, 15, 220});
			DrawRectangle(hx, ry, (int)(hw * rf), 14, rage.berserk ? Color{255, 170, 40, 255} : Color{210, 120, 40, 255});
			if (rage.berserk)
				DrawText("BERSERK", hx + hw + 10, ry - 3, 20, Color{255, 170, 40, 255});
			DrawText(TextFormat("Coins %d    Keys %d    Potions %d", itemCount(inventory, kItemCoin), itemCount(inventory, kItemKey), itemCount(inventory, kItemHealthPotion)), hx, ry - 30, 20, Color{230, 220, 185, 255});
			DrawText(TextFormat("Weapon: %s  [%s to swap]", weaponName(equippedItem), codeName(actionCode(keys, Action::NextWeapon)).c_str()), hx, ry - 52, 20, Color{200, 220, 235, 255});
			if (skills.points > 0)
				DrawText(TextFormat("Skill points: %d  [%s]", skills.points, codeName(actionCode(keys, Action::SkillMenu)).c_str()), hx, ry - 74, 20, Color{235, 220, 130, 255});
			if (leverTarget >= 0 || doorTarget >= 0 || chestTarget >= 0) // unified "use" prompt
			{
				const char* msg = "[E] Pull lever";
				bool warn = false;
				if (leverTarget < 0 && doorTarget >= 0)
				{
					warn = doors[doorTarget].locked && itemCount(inventory, kItemKey) == 0;
					msg = warn ? "[E] Locked - need a key" : (doors[doorTarget].locked ? "[E] Unlock door" : "[E] Open door");
				}
				else if (leverTarget < 0 && chestTarget >= 0)
				{
					warn = containers[chestTarget].locked && itemCount(inventory, kItemKey) == 0;
					msg = warn ? "[E] Locked - need a key" : (containers[chestTarget].locked ? "[E] Unlock" : "[E] Open");
				}
				DrawText(msg, 520, 470, 24, warn ? Color{230, 120, 110, 255} : Color{240, 235, 210, 255});
			}
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
