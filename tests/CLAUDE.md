# tests — CLAUDE.md

Headless doctest suite over `adventure_lib`. **Never opens a window** — so it runs anywhere (local + CI).

| File | Covers |
|------|--------|
| `test_main.cpp` | doctest entry point (`DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN`) — no cases. |
| `test_scriptengine.cpp` | Lua sandbox (dangerous libs absent), watchdog, bytecode/syntax rejection, memory reporting. |
| `test_metrics.cpp` | Section timing with an injected clock (deterministic), per-frame reset, fps derivation. |
| `test_profilereport.cpp` | `computeFrameStats` avg/min/max/percentiles, empty input, input-copied. |

## Rules
- Test **logic**, not rendering — anything needing a GL window is verified by running the game, not here.
- Make time/IO injectable so cases are deterministic (see `Metrics::setClock`). No sleeps, no wall-clock.
- New behaviour in `adventure_lib` that's headlessly testable ships with a test (required by `adv-feature`).
- Add `test_<name>.cpp` and list it in the `adventure_tests` target in `CMakeLists.txt`.
