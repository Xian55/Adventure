# Development Loop — how a feature request becomes shipped code

The goal: you request a feature; it gets **designed, implemented under SRP, thoroughly tested, gated on
performance, verified, and committed** — reliably and with minimal back-and-forth. This is the harness that
makes that autonomous. The canonical procedure lives in the **`adv-feature`** skill; this doc explains the
machinery it drives and the guarantees it gives.

## The loop
```
request → acceptance criteria → explore/reuse → SRP design → tests (as you go)
        → implement (+Metrics::Scope on hot paths) → GATES → self-review → commit
```

## The gates (a feature isn't done until all pass)
| Gate | Command | Guarantee |
|------|---------|-----------|
| **Unit tests** | `adv-test` / `ctest ... mingw-debug` | Logic is correct and stays correct (headless doctest over `adventure_lib`). |
| **Perf gate** | `ctest ... mingw-release -C Release` (runs `adventure_bench`) | No hot path exceeds its ms budget; regressions fail the build. |
| **Loop profile** | `ADVENTURE_PROFILE=300 ./adventure.exe` → `profile.csv` | Frame avg/p95 stays within budget; compare before/after. |
| **Visual verify** | `adv-build` (screenshot) | It actually renders / looks right. |
| **CI** | GitHub Actions on push/PR | The above build+test+perf gate is enforced on the server too. |

## Performance is a first-class constraint
- **Measure, don't guess.** Instrumented from day one: `Metrics` (F3 overlay) times every section; wrap any
  new per-frame/per-entity work in `Metrics::Scope s(metrics, "name")`.
- **Budget, don't hope.** Hot algorithms get a `bench/` case with a ms ceiling. The gate fails on breach.
- **Profile the real loop.** `ADVENTURE_PROFILE=<frames>` runs uncapped and writes frame avg/p50/p95/max +
  RSS + Lua bytes + per-section ms to `profile.csv` — no human required.
- **Frame budget:** 60 fps = 16.6 ms/frame. We currently sit far under (sub-millisecond at M0), by design
  (retro-cheap rendering). The job of the gates is to keep that headroom as features land.

### Current baseline (M0, mingw-release, empty scene)
- Frame: sub-millisecond uncapped (thousands of fps); scene ~0.05 ms, post ~0.03 ms.
- `Metrics::Scope` overhead ≈ 12 ns; Lua chunk compile+run ≈ 2–4 µs. Instrumentation is effectively free.
(Re-measure with `ADVENTURE_PROFILE` / `adventure_bench` after significant features and update this.)

## What stays testable
Game logic lives in `adventure_lib`; `adventure_tests` never opens a window. Keep time/IO **injectable**
(e.g. `Metrics::setClock`, pass clocks/paths in) so tests are deterministic. Rendering and *feel* are
verified by running the game and by iterating Lua tuning — not by unit tests.

## Skills that automate the steps
- **`adv-feature`** — the full loop above (use on every feature request).
- **`adv-new-module`** — scaffold an SRP `.h/.cpp` (+ test) wired into the build.
- **`adv-test`** — build + run unit tests, and the Release perf gate.
- **`adv-build`** — build + run + screenshot (with the DPI crop) to verify visually.

## How to request a feature
Describe the feature and its intent (what it should feel like / do). Optionally name a milestone. The loop
handles the rest and reports back: what was designed, the tests added, the before/after perf numbers, and a
screenshot if it renders. Ambiguities that change the design get one focused question; everything else uses
a sensible, stated default.
