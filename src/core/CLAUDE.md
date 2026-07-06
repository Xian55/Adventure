# src/core — CLAUDE.md

Cross-cutting foundations. No gameplay here.

| File | Responsibility |
|------|----------------|
| `Config.h` | Compile-time constants (window/low-res dims, fixed timestep). Frequently-tuned values go in Lua, not here. |
| `Platform.{h,cpp}` | OS process metrics (working set, CPU%). **No raylib** (isolates `<windows.h>`). Non-Windows = stubs. |
| `Metrics.{h,cpp}` | Per-frame perf data: section timers (RAII `Scope`), frame-time + spike max, RSS/CPU/Lua-bytes snapshot. **No raylib**; clock injectable → headless-testable. Drawn by `render/MetricsOverlay`. |
| `ProfileReport.{h,cpp}` | Headless profiling-run stats + CSV. `computeFrameStats` is pure (tested). Used by the `ADVENTURE_PROFILE` path in `main`. |

## Rules
- Keep this dir raylib-free except where unavoidable; presentation lives in `render/`.
- `Metrics` is a normal constructable class (tests make their own) with a static `instance()` for the game.
- Anything measuring must not itself be measurable overhead: `Metrics::Scope` is ~12 ns (see `bench/`).

## Extending
Time/IO must be injectable for tests (see `Metrics::setClock`). New per-frame data → add to `Metrics`,
surface in `MetricsOverlay`, and add to `ProfileReport`'s CSV.
