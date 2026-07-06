# bench — CLAUDE.md

The performance gate. Headless micro-benchmarks; `adventure_bench` returns nonzero if any case exceeds its
ms budget. Registered in CTest for **Release only** (Debug is too noisy).

| File | Responsibility |
|------|----------------|
| `Bench.h` | Header-only harness: `ADV_BENCH(name, iters, budgetMs) { ...one iteration... }`, `Registry::runAll()`, `keep()` to defeat dead-code elimination. |
| `bench_lua.cpp` | Lua call-path costs (runString throughput, VM heap query). |
| `bench_metrics.cpp` | Metrics scope/frame overhead (must stay negligible). |

## Rules
- Budgets are **ceilings with headroom** (measured, then tripled), not targets — they catch regressions.
- Run Release for real numbers: `./build/mingw-release/adventure_bench.exe`.
- **Add a case for any new hot-path code** (per-frame / per-entity algorithms). This is required by the
  `adv-feature` loop.

## Current baseline (mingw-release)
`Metrics::Scope` ≈ 12 ns · Lua chunk compile+run ≈ 2–4 µs · Lua heap query ≈ 14 ns. Re-measure after big
features and update `docs/design/DEVELOPMENT_LOOP.md`.
