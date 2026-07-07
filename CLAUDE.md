# CLAUDE.md — Adventure

GRAVEN-style first-person dark-fantasy melee game. raylib 5.5 + C++17, gameplay scripted in sandboxed Lua.
Retro look = low-res render + point upscale + fog + dither. Status: **M0 + M1 done** — `.map` parse +
geometry/collision (`src/world`), world rendering (`render/WorldRenderer`), Quake first-person movement +
AABB-vs-brush collision (`src/player`), torch+sword viewmodel (`render/Viewmodel`), Lua feel-tuning. **Next:
M2 combat slice** (melee state machine + sweeping hitbox, kick, shield, one skeleton). Controls: WASD +
mouse-look, Shift sprint, Space jump, Ctrl crouch, F3 metrics, F4 training telemetry (jump dist/height +
player dims). Default map `maps/training.map` (jump staircase + crouch bars); override with `ADVENTURE_MAP`;
`tools/gen_room.py`/`gen_training.py` emit maps.

## Commands
```bash
cmake --preset mingw-debug && cmake --build build/mingw-debug      # build (also: msvc-debug)
ctest --test-dir build/mingw-debug --output-on-failure             # unit tests (headless)
cmake --preset mingw-release && cmake --build build/mingw-release
ctest --test-dir build/mingw-release -C Release --output-on-failure # tests + perf gate (bench)
./build/mingw-debug/adventure.exe                                   # run (F3 = metrics overlay)
ADVENTURE_PROFILE=300 ./adventure.exe                               # headless profile -> profile.csv
ADVENTURE_SHOT=shot.png ./adventure.exe                             # one-shot screenshot, then exit
```
Skills automate these: `adv-feature` (full loop), `adv-build`, `adv-test`, `adv-new-module`, `adv-format`,
`adv-docs`, `adv-map` (.map format authority), `adv-sprite` (Aseprite export), `adv-onboard` (new-contributor
setup).

## Layout
`src/` game code (built into `adventure_lib`; thin `src/main.cpp`) — subsystems `core/ render/ lua/ world/
player/` · `tests/` doctest · `bench/` perf gate · `scripts/` Lua · `maps/` `.map` levels (`tools/gen_room.py`
emits them) · `assets/` local PNGs (gitignored) · `deps/` vendored minilua + LuaBridge · `docs/design/` the
design docs (+ `BACKLOG.md`). Each `src/<subsystem>/` has its own `CLAUDE.md`.

Tooling env vars: `ASEPRITE` (Aseprite.exe path — used by `adv-sprite`).

## Conventions (enforced — see docs/design/CONVENTIONS.md)
- **Style is machine-enforced** by `.clang-format` (CI gate). Tabs, Allman (lambdas inline), `m_` members,
  PascalCase types, `k`-constants, left pointers, `ColumnLimit: 0` (never auto-wrap — break lines by hand).
- **One record per line** for multi-element vector/array/string-literal lists (trailing comma).
- **Lean comments** — one line stating *why*, never walls of text. Put the longer explanation in the
  subsystem `CLAUDE.md`, not in the code.
- **SRP** — one module, one job (data / presentation / OS / policy split). Keep `entt::` out of
  render/world/lua public headers and out of Lua.
- **Instrumented** — wrap per-frame/hot work in `Metrics::Scope`; hot algorithms get a `bench/` case.

## Gotchas
- MinGW binaries statically link the GCC/libstdc++/winpthread runtimes (fixes ctest `0xc0000139`).
- raylib `TakeScreenshot` double-scales on HiDPI; `adv-build` crops it.
- CI builds with Ninja + `msvc-dev-cmd` (the VS generator can't find VS on the runner image).
- Lua linter flags `tuning`/`host` as undefined globals in `scripts/` — false (sandbox injects them).

## Working agreement
Keep docs current as code changes: update the relevant **subsystem `CLAUDE.md`**, this file, and
`docs/design/` when architecture/commands/conventions shift (use the `adv-docs` skill; it's the last step
of `adv-feature`). Features go through the `adv-feature` loop: design → tests → perf gate → verify → commit.
