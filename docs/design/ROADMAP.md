# Roadmap

Milestones are vertical and feel-driven. Each should end in something runnable and, where it has runtime
behaviour, verifiable end-to-end (run the game / run the tests), not just compiled.

| # | Milestone | Goal | Status |
|---|-----------|------|--------|
| **M0** | Scaffold + pipeline | CMake (raylib/EnTT/Lua wired, MinGW+MSVC), window, fixed-step loop, low-res RT + point upscale + palette/dither post, Lua sandbox boots + `selfTest`, **instrumentation (F3 metrics)**, **test harness (doctest)**. | ✅ done |
| **M1** | Movement + world | `.map` (TrenchBroom) parse → brush meshes + collision planes, Quake-accel first-person controller, AABB-vs-brush collision, torch+sword viewmodel. *Walk a pixelated room with fluid movement.* | ✅ done — parse, geometry/collision, world render, Quake movement + collision (walk/sprint, jump/crouch), torch+sword viewmodel, Lua feel-tuning |
| **M2** | Combat slice (**target**) | Melee swing state machine + sweeping hitbox, kick knockback, shield block, one skeleton (approach/attack/stagger/die), arena script, `trigger_hurt` hazard kill. **Nail the feel.** | in progress — M2a directional melee + hitbox + sword anim ✓; M2b skeleton (approach/stagger/die) + hit resolution ✓; M2c kick ✓, shield block + enemy strike-back + player health/respawn ✓. Left: rage→berserk meter, billboard sprites, arena/`trigger_hurt` script |
| M3 | Inventory + pickups + HUD | Weapon/item pickups, hotbar/equip, Lua-driven HUD. | |
| M4 | RPG stats | Health/stamina/attributes, damage formulas, loadout. | |
| M5 | Ranged + spells | Crossbow projectiles, data-driven spell system. | |
| M6 | Quests + triggers | `trigger_*` → Lua events, objectives, doors/keys, puzzles. | |
| M7 | Arenas + secrets | Encounter waves, hidden areas, verticality. | |
| M8 | Content breadth | More enemies/weapons/maps; swap billboard enemies → 3D animated models behind the `RenderKind` seam; **Recast/Detour navmesh pathfinding** as levels grow beyond one room (see ARCHITECTURE). | |
| M9 | Multiplayer | Co-op authority model (binary protocol + snapshot interpolation). | |

## Rendering decision (locked)
**Hybrid**: full 3D world geometry + collision; enemies/pickups start as **2D billboard sprites**
(Doom-style) behind a `RenderKind` abstraction, swappable to 3D animated models later without engine
changes. Chosen for fast art iteration now.

Captured feature requests (not yet scheduled) live in [BACKLOG.md](BACKLOG.md): chests/loot/keys (M3),
crafting, Dark Messiah–style skill points (M4).

## Principles carried through every milestone
- Feel constants live in `scripts/tuning.lua` (hot-reload), not in C++.
- New runtime behaviour ships with a test where it's headlessly testable.
- Keep the `entt::` registry out of render/world/lua public seams — pass plain data.
