# Conventions

## Single Responsibility
One module, one job. Split **data / presentation / OS / policy**:
- Data collectors don't draw (`Metrics` vs `MetricsOverlay`).
- OS calls are isolated (`Platform`) so they never leak `<windows.h>` into raylib TUs.
- The Lua binding library lives in exactly one TU (`Bindings.cpp`).
- Keep `entt::` out of render/world/lua public headers.

If a file needs "and" to describe what it does, consider splitting it.

## Performance
Real-time engine — see **[PERFORMANCE.md](PERFORMANCE.md)** for hot-path coding rules (no per-frame
allocation, data-oriented iteration, static dispatch, batch draws, measure-don't-guess). Enforced by the
bench budgets + `profile.csv`.

## Code style (C++)
**Machine-enforced** by `.clang-format` (root) and gated in CI — run `adv-format` before committing.
Warnings are errors on our targets (`-Wall -Wextra -Werror` / `/W4 /WX`); deps are exempt (SYSTEM/separate).
clang-format 19.x; `deps/` is excluded. The rules it encodes:
- **Tabs** for indent; **Allman braces** (opening brace on its own line) — but **lambda bodies stay inline**.
- `ColumnLimit: 0` — never auto-wrap; break long lines by hand where they read best.
- `PascalCase` types, `camelCase` methods/functions, `m_` member prefix, `k`-prefixed constants.
- Left-aligned pointers/refs (`int* p`, `int& r`). `#pragma once`. `std::unique_ptr`/RAII; pimpl for heavy deps.
- Namespace everything in `adventure` (or `adventure::sub`). Singletons via an `sX` macro (`sScript`) *only*
  where a single instance is genuinely global — prefer plain constructable classes (`Metrics`) so tests can
  make their own.
- **One record per line** for multi-element vector/array/string-literal initializer lists, with a trailing
  comma. clang-format preserves this but won't auto-explode a packed list — write it exploded.

## Comments & docs
- **No walls of comment text.** A code comment is one line stating the *why*. If it needs a paragraph, it
  belongs in the subsystem `CLAUDE.md`, not in the code.
- Every `src/<subsystem>/` has a `CLAUDE.md` (file table + gotchas + how-to-extend) — the map that speeds up
  understanding. Keep it and the root `CLAUDE.md` current as code changes (use `adv-docs`).
- Cross-link: comments point to the subsystem `CLAUDE.md`; `CLAUDE.md` points to `docs/design/`.

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
