# Conventions

## Single Responsibility
One module, one job. Split **data / presentation / OS / policy**:
- Data collectors don't draw (`Metrics` vs `MetricsOverlay`).
- OS calls are isolated (`Platform`) so they never leak `<windows.h>` into raylib TUs.
- The Lua binding library lives in exactly one TU (`Bindings.cpp`).
- Keep `entt::` out of render/world/lua public headers.

If a file needs "and" to describe what it does, consider splitting it.

## Code style (C++)
Matches the author's house style:
- **Tabs** for indent; **Allman braces** (opening brace on its own line).
- `PascalCase` types, `camelCase` methods/functions, `m_` member prefix, `k`-prefixed constants.
- `#pragma once`. Prefer `std::unique_ptr`/RAII; pimpl to hide heavy deps from headers.
- Namespace everything in `adventure` (or `adventure::sub`). Singletons via an `sX` macro
  (`sScript`) *only* where a single instance is genuinely global — prefer plain constructable classes
  (like `Metrics`) so they stay testable.
- Comments explain **why**, not what.

## Testing
- Logic lives in `adventure_lib` so `adventure_tests` (doctest) can exercise it **without a window**.
- Make time/IO **injectable** (see `Metrics::setClock`) so tests are deterministic, not flaky.
- Every new runtime behaviour that *can* be tested headlessly ships with a test. Rendering is verified by
  running the game and screenshotting (see the `adv-build` skill), not by unit tests.
- Run: `ctest --test-dir build/mingw-debug --output-on-failure` (or the `adv-test` skill).

## Data-driven feel
Tunable constants and content (weapons, enemies, encounters, UI) go in **sandboxed Lua** under `scripts/`,
hot-reloadable. C++ owns the hot path; Lua owns the numbers and the policy.

## Build & run
- Configure once per preset: `cmake --preset mingw-debug` (also `msvc-debug`).
- Build: `cmake --build build/mingw-debug`.
- Both toolchains must stay green. MinGW binaries statically link the GCC/libstdc++/winpthread runtimes.

## Assets
See [../ASSET_PIPELINE.md](../ASSET_PIPELINE.md). Loose PNGs, fixed palette, strict naming so TrenchBroom
and the engine agree. Automate generation; never hand-place what a script can stamp.
