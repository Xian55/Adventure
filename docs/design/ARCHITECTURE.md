# Architecture

raylib 5.5 + C++17. Windows (MinGW + MSVC), CMake + presets. Single-responsibility modules; the game
logic lives in a static library (`adventure_lib`) so it is unit-testable without a window.

## Build layout
```
adventure_lib (STATIC)  ← all systems; links raylib, EnTT, psapi
   ├─ core/     Config, Platform (OS mem/cpu), Metrics (perf data)
   ├─ render/   Renderer (low-res pipeline), MetricsOverlay, Shaders   [+ Billboard, Viewmodel, Frustum]
   ├─ lua/      ScriptEngine (pimpl), Bindings.cpp (only TU that sees LuaBridge)
   └─ deps/     minilua (Lua 5.5, one C TU), luabridge (header)        [+ ecs/, world/, combat/ as they land]
adventure (EXE)         ← thin src/main.cpp; links adventure_lib
adventure_tests (EXE)   ← doctest over adventure_lib (headless; never opens a window)
```
Dependencies fetched via CMake `FetchContent`: **raylib 5.5**, **EnTT v3.13.2**, **doctest v2.4.11**.
Lua/LuaBridge are vendored under `deps/`. See `CMakeLists.txt` and `cmake/deps.cmake`.

## Single Responsibility — the seams we hold
- **Platform** talks to the OS (working set, CPU time). No raylib, no formatting.
- **Metrics** measures and aggregates (timing, memory snapshot). No raylib, injectable clock → testable.
- **MetricsOverlay** *draws* a Metrics snapshot. Presentation only; never mutates.
- **Renderer** owns the RT pipeline: `beginScene` → (caller draws) → `endScene` → `postProcess` → `blit`.
  `main` wraps `blit` in `BeginDrawing`/`EndDrawing` so it can draw the native-res overlay on top.
- **ScriptEngine** owns the Lua VM + sandbox behind a pimpl. Only `Bindings.cpp` includes LuaBridge.

## ECS (EnTT) — arrives M1/M2
Confine `entt::` to `src/ecs/`. Systems are functions taking `(registry&, dt, WorldContext&)`. Do **not**
leak `entt::` types into render/world/lua public headers or into Lua — pass plain data across seams.

Planned components (combat slice): `Transform`, `Velocity`, `Health`, `Collider(AABB)`,
`PlayerController`, `WeaponState`, `ShieldState`, `EnemyAI`, `Billboard`, `Faction`, `Hurtbox`, `Hitbox`,
`LuaScriptRef`, `LightSource`.

Fixed-timestep update order (60 Hz; render at display rate):
`Input → PlayerMove → AI → Combat → Physics → Render`. Lua `onFrame(dt)` (arena scripts) runs between AI
and Combat.

## Rendering pipeline
Scene → **low-res RenderTexture** (480×270) → **post RT** (palette quantize + Bayer 4×4 dither, indexed by
low-res texel so the dither stays coarse) → **point-filter upscale** to the framebuffer. World fog is
exp²; a dark gothic clear colour. Note the RenderTexture Y-flip (negative source height) and DPI: blit to
`GetRenderWidth/Height`, not the logical window size, or you get black borders on scaled displays.

Billboards (enemies): Y-axis camera-facing quads, frustum-culled then depth-sorted, point-filtered.
`RenderKind` on the entity selects billboard now / 3D model later — swapping touches only `RenderSystem`.

## Lua boundary (sandbox)
minilua (Lua 5.5) driven through the raw C API for the core (state, whitelisted `_ENV`, instruction-count
watchdog, chunk loading as text-only to reject bytecode, `pcall` isolation). LuaBridge3 marshals the
game API and is confined to `Bindings.cpp` (compiled with big-obj). Untrusted scripts never see `_G`,
`io`, `os.execute`, `require`, `load`, `dofile`, `debug`, or `package`. The bridge to gameplay is narrow
and value-based: Lua AI gets a handle table backed by bound C functions over a stable `entt::entity`.

## Instrumentation
`Metrics` (F3 overlay): FPS, frame ms, rolling max (spike detector), process CPU %, RSS, Lua VM bytes, and
per-section CPU time via RAII `Metrics::Scope`. Wrap any suspect block in a scope to measure it:
```cpp
{ Metrics::Scope s(metrics, "ai"); runAI(...); }
```

## Pathfinding (planned) — Recast & Detour
**Decision:** enemy navigation uses **recastnavigation** (Recast for navmesh generation, Detour for runtime
queries), added via `FetchContent`. Bake a navmesh from the level triangle soup (`BrushGeometry`'s render
meshes) at map load, with agent radius/height/climb derived from the player/enemy collider dims. `AISystem`
queries `dtNavMeshQuery` (findPath / wall-follow) and feeds the corridor to enemy steering. Throttle queries
per enemy (`thinkTimer`), cache the path, and wrap the query in `Metrics::Scope("ai.path")` with a bench
budget. Lands when enemies need real navigation (multi-room / obstacle avoidance, ~M6); the M2 skeleton uses
simple direct steering in one room and needs no navmesh.

## Levels (TrenchBroom `.map`) — arrives M1
Quake **Valve 220** brushes = convex intersections of face half-spaces. Parse → clip faces → per-texture
meshes (render) + face planes (collision). Entities (`info_player_start`, `monster_skeleton`,
`weapon_sword`, `light`, `trigger_hurt`, `func_*`) map to ECS spawns via a `classname → spawn` table.
Convert Z-up (map) → Y-up (raylib) in one place. Author `adventure.fgd` so the editor offers our entities.
