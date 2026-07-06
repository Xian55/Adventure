# Adventure

A first-person, melee-focused **dark-fantasy** action game in the style of **GRAVEN** — Quake-fluid
movement, Dark Messiah–style combat (directional melee, kick, shields), rendered *retro* (low internal
resolution + point upscale + fog + dither). Built on **raylib 5.5 + C++17**, gameplay scripted in
sandboxed **Lua**.

## Status
**M0 complete** — build wiring (MinGW + MSVC), low-res render pipeline, sandboxed Lua VM, F3 performance
instrumentation, and a headless test suite. See [docs/design/ROADMAP.md](docs/design/ROADMAP.md).

## Quick start
```bash
# configure (fetches raylib 5.5 + EnTT + doctest on first run)
cmake --preset mingw-debug          # or: msvc-debug

# build
cmake --build build/mingw-debug

# run
./build/mingw-debug/adventure.exe   # F3 toggles the metrics overlay

# test
ctest --test-dir build/mingw-debug --output-on-failure
```

Requirements: CMake ≥ 3.24, a C++17 compiler (WinLibs MinGW-w64 UCRT or MSVC 2022), git, network on first
configure. Ninja recommended for the MinGW preset.

## Documentation
- [Vision](docs/design/VISION.md) — what we're making and the feel we're chasing.
- [Architecture](docs/design/ARCHITECTURE.md) — modules, ECS/Lua/render seams, build layout.
- [Roadmap](docs/design/ROADMAP.md) — milestones (M0…M9).
- [Combat](docs/design/COMBAT.md) — melee/kick/shield/enemy design and the C++/Lua split.
- [Conventions](docs/design/CONVENTIONS.md) — SRP, code style, testing, data-driven feel.
- [Asset pipeline](docs/ASSET_PIPELINE.md) — producing retro assets at scale.

## Repo layout
```
src/        game code (built into adventure_lib; thin src/main.cpp)
tests/      doctest unit tests (headless)
scripts/    sandboxed Lua: tuning, weapons, enemies, arenas
maps/       TrenchBroom .map levels (from M1)
assets/     textures, sprites, viewmodel (loose PNG, fixed palette)
deps/       vendored minilua (Lua 5.5) + LuaBridge3
cmake/      dependency wiring
docs/       design docs + asset pipeline
.claude/    project automation skills (adv-build, adv-test, adv-new-module)
```

## Automation (Claude skills)
Project skills streamline the repetitive loops: `adv-build` (build + run + screenshot + DPI-crop),
`adv-test` (build + ctest), `adv-new-module` (scaffold an SRP `.h/.cpp` pair). See `.claude/skills/`.
