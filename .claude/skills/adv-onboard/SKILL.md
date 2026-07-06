---
name: adv-onboard
description: Bootstrap a new contributor (or a fresh machine) into the Adventure project — install the toolchain, set environment variables, build, and verify everything works. Use when someone is setting up the project for the first time, or when a clean-machine setup / environment problem needs fixing.
---

# adv-onboard — set up the Adventure project from scratch

Goal: from nothing to a green build + running game. Windows is the primary target. Do the steps in order;
each ends in a check.

## 1. Prerequisites (install if missing)
- **git** ≥ 2.4, **CMake** ≥ 3.24, **Ninja** ≥ 1.11.
- A C++17 toolchain — **either**:
  - **MinGW-w64 UCRT** (WinLibs) — `winget install BrechtSanders.WinLibs.POSIX.UCRT` (gcc on PATH), **or**
  - **Visual Studio 2022** (MSVC, Desktop C++ workload).
- Optional (asset work): **Aseprite** (sprites), **TrenchBroom** (levels).
Verify: `cmake --version`, `git --version`, and `gcc --version` (MinGW) or a VS Developer prompt (MSVC).

## 2. Clone
```bash
git clone https://github.com/Xian55/Adventure.git
cd Adventure
```
Note: `assets/` binaries are gitignored (kept local); the build still works (only the `.gitkeep` skeleton
is tracked).

## 3. Environment variables (User scope, persistent)
Set the tool paths the skills reference — adjust to your install locations:
```powershell
# Aseprite (only if you'll export sprites) — used by adv-sprite / the asset pipeline
[Environment]::SetEnvironmentVariable("ASEPRITE","C:\Program Files\Aseprite\Aseprite.exe","User")
```
Open a **new** shell so the variables take effect. Verify: `$env:ASEPRITE`.
(See `memory`/`docs` for the current canonical list; keep this in sync when a new tool env var is added.)

## 4. Build (first run fetches raylib 5.5 + EnTT + doctest — needs network, a few minutes)
```bash
cmake --preset mingw-debug         # or: msvc-debug
cmake --build build/mingw-debug
```

## 5. Verify — all three must pass
```bash
# unit tests (headless)
ctest --test-dir build/mingw-debug --output-on-failure

# perf gate (Release)
cmake --preset mingw-release && cmake --build build/mingw-release
ctest --test-dir build/mingw-release -C Release --output-on-failure

# run the game (F3 = metrics overlay)
./build/mingw-debug/adventure.exe
```
Expected: tests + bench green; a window shows a pixelated, fogged scene; console prints
`ScriptEngine selfTest OK`.

## 6. Editor / formatting
- Install **clang-format 19** (VS ships one; or `pip install clang-format==19.1.5`) so `adv-format` and the
  CI format gate agree. Enable "format on save" pointed at the repo `.clang-format` if your editor supports it.
- `.editorconfig` is respected by most editors (tabs, LF, final newline).

## 7. Learn the project (read in this order)
- `CLAUDE.md` (root) — commands, layout, conventions, gotchas.
- `docs/design/VISION.md`, `ARCHITECTURE.md`, `ROADMAP.md`, `DEVELOPMENT_LOOP.md`.
- The subsystem `src/<name>/CLAUDE.md` for whatever you'll touch.
- Skills: `adv-feature` (the dev loop), `adv-test`, `adv-build`, `adv-new-module`, `adv-format`, `adv-docs`,
  `adv-map`, `adv-sprite`.

## Troubleshooting
- `0xc0000139` running tests → wrong MinGW runtime DLL on PATH; the static link should prevent it (rebuild).
- CMake can't find a compiler → use a preset matching your toolchain (`mingw-*` vs `msvc-*`); for MSVC run
  from a Developer prompt or via `vcvars64.bat`.
- First configure fails offline → the dep fetch needs network once; then it's cached.
