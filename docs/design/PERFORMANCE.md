# Performance — coding guidelines

Adventure is a real-time engine with manual memory, a hot per-frame/per-entity path, and a Lua boundary.
These are the rules that keep it fast *as it grows*. They're concrete to this codebase, not generic advice.
Tooling that enforces them: F3 `Metrics`, `bench/` budgets, `ADVENTURE_PROFILE` (see `DEVELOPMENT_LOOP.md`).

## The budget
- **60 fps = 16.6 ms/frame.** We currently sit **sub-millisecond** (retro-cheap by design). The job of
  every change is to *keep that headroom*, not to hit 16 ms.
- The perf gate (`adventure_bench`) fails CI on regression. Loop cost is visible in `profile.csv`.

## Golden rule: measure, don't guess
- Wrap any suspect per-frame/per-entity block in `Metrics::Scope s(metrics, "name")` — it costs ~12 ns.
- Add a `bench/` case with a budget for any hot *algorithm* (collision query, brush meshing, Lua call).
- Profile before/after (`ADVENTURE_PROFILE=300`). Optimize only what the numbers say is hot.
- Do **not** micro-optimize cold code (map load, setup) — clarity wins there.

## The hot path (per-frame / per-entity)
- **No heap allocation in the frame loop.** No per-frame `std::string`/`std::vector` churn. Reserve once,
  reuse buffers, keep scratch containers as members. (Map load / level build may allocate freely — it's cold.)
- **Data-oriented.** When ECS lands, iterate EnTT **views** over contiguous component arrays; don't chase
  pointers. Keep hot components small and POD so they stay cache-friendly.
- **Static dispatch in inner loops.** Avoid `virtual` / `std::function` per entity per frame; prefer plain
  functions, enums+switch, or templates. (`std::function` is fine for setup/config, not the tick.)
- **Pass non-trivial types by `const&`**, return by value only for small PODs.
- **Plain math** (raymath `Vector3*`) over heavy abstractions in hot code. No hidden allocations/copies.
- Branch on the common case first; hoist invariant work out of loops.

## Rendering
- **Batch by texture** (WorldRenderer already merges per-texture meshes) — minimize draw calls.
- **Upload static geometry once** (`UploadMesh` at load); never rebuild meshes per frame.
- **Cull** before drawing (per-mesh frustum cull is the next win; then BVH/portals — see ARCHITECTURE
  "World space partitioning"). Don't submit off-screen geometry.
- The low-res `RenderTexture` is the biggest perf lever — keep the internal resolution low; that's the look.

## Collision / world
- **Broadphase before narrowphase.** `CollisionWorld::overlaps` already AABB-filters before the plane test.
- The brush list is **flat** today (fine at M1 scale). Add a BVH/uniform grid **when the profiler shows the
  O(n) scan costing** — not before. Big maps (Arcane Dimensions, 3000+ brushes) need this to be *playable*
  even though they *load* fine.

## Lua boundary
- A Lua call costs **~2–4 µs** (compile+run) / ~ns for a bound function — real if done per entity per frame.
- **Load definitions once** into C++ structs at map/entity spawn (weapon/enemy defs, tuning). Don't re-read
  Lua tables every frame.
- Per-frame AI ticks call **small bound methods** over a stable `entt::entity`; marshal narrow value data,
  never big tables. Keep the C++↔Lua boundary in one TU (`Bindings.cpp`).

## Memory
- Prefer stack / member scratch over transient heap. The Lua VM is hard-capped (64 MB allocator).
- Watch RSS in the F3 overlay; a per-frame RSS climb means a leak or per-frame allocation.

## Determinism (cheap to keep, expensive to retrofit)
The sim is fixed-step; movement/collision take `dt` (no wall-clock). If a per-frame optimization introduces
float order-of-operations or iteration-order dependence, note it — it matters for future co-op (M9). ASan/
UBSan CI catches the uninitialized-read class that also breaks determinism.

## What the gates already catch (so you don't have to hand-check)
- **-Werror** (`-Wall -Wextra` / `/W4 /WX`): sign-conversion, uninitialized fields, unused results.
- **ASan/UBSan** CI: use-after-free, OOB, UB (caught our dangling-ref instantly).
- **bench budgets**: perf regressions. **golden geometry**: silent geometry/coord-conversion shifts.

## Quick checklist for a hot-path change
1. Wrapped in a `Metrics::Scope`? 2. Any per-frame allocation removed? 3. Bench case + budget added?
4. `profile.csv` before/after within budget? 5. Cold vs hot split correct (setup out of the loop)?
