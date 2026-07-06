---
name: adv-feature
description: The end-to-end harness for building a requested feature in Adventure autonomously — design it, implement it under SRP, test it thoroughly, gate it on performance, verify it, and commit. Use this whenever the user requests a new gameplay/engine feature (a system, mechanic, weapon, enemy, level element, etc.) rather than a trivial fix. It defines the required steps and quality gates so the work is reliable and doesn't regress performance.
---

# adv-feature — the autonomous feature loop

When the user requests a feature, follow this loop. Do not skip the gates. The point is that a feature
request turns into working, tested, non-regressing code without hand-holding.

## 1. Frame it as acceptance criteria
Restate the request as a short list of **observable** "done" conditions (what the player/tester can see or
what a test can assert). Note which milestone it belongs to (`docs/design/ROADMAP.md`). If a requirement is
genuinely ambiguous *and* the choice changes the design, ask one focused question; otherwise pick the
sensible default and state it.

## 2. Explore before writing
- Search for existing code to reuse (systems, math, patterns). Prefer reuse over new code.
- Identify the **SRP seam**: which module owns this, or what new module(s) are needed. Keep data /
  presentation / OS / policy separate (see `docs/design/CONVENTIONS.md`).
- Identify the **C++ vs Lua split**: hot path / core logic in C++, numbers + policy in Lua (`scripts/`).
- Identify the **performance surface**: does it run per-frame or per-entity? If yes, it needs a
  `Metrics::Scope` around it and (if it's a hot algorithm) a bench case.

## 3. Design (write it down)
State the module list, key data structures, the update-order placement (Input→PlayerMove→AI→Combat→
Physics→Render), and any new components. Keep `entt::` out of render/world/lua public headers and out of
Lua. For anything non-trivial, use the `adv-new-module` skill to scaffold each SRP unit.

## 4. Test thoroughly (this is not optional)
Write tests **as you implement**, not after. Cover:
- **Happy path** — the feature does what the criteria say.
- **Edges** — empty/zero/max inputs, boundaries (e.g. exactly at hitbox edge, 0 hp, first/last frame).
- **Failure modes** — bad script, missing entity, out-of-range — must fail cleanly, not crash.
- Put logic in `adventure_lib` so tests are **headless**; make time/IO injectable (see `Metrics::setClock`)
  for determinism. Rendering/feel is verified by running the game, not unit tests.
Add `tests/test_<name>.cpp`, list it in `adventure_tests` in `CMakeLists.txt`.

## 5. Implement
Match house style (tabs, Allman, `m_`, PascalCase). Wrap any per-frame/hot block in
`Metrics::Scope s(metrics, "<name>")` so its cost shows in the F3 overlay and profile runs.

## 6. Gates — all must pass before commit
Run in order; fix and re-run on any failure:
1. **Unit tests** — `adv-test` (or `ctest --test-dir build/mingw-debug --output-on-failure`). Green.
2. **Perf gate** — if the feature touches a hot path, add a bench case in `bench/` with a budget, then
   build Release and run `ctest --test-dir build/mingw-release -C Release` (includes `adventure_bench`).
   No case over budget.
3. **Profile the loop** — if it runs every frame, profile before/after:
   `ADVENTURE_PROFILE=300 ./adventure.exe` writes `profile.csv`. Frame `avg`/`p95` must stay within the
   frame budget (target ≤ 16.6 ms for 60 fps; we currently sit far under — so mainly watch for a
   *regression* vs the previous `profile.csv`). Report the before/after numbers.
4. **Visual verify** — if it renders or changes feel, use `adv-build` to screenshot and confirm it looks
   right. For feel, iterate constants in `scripts/tuning.lua` (hot-reload), not C++.

## 7. Self-review, then commit
Check: correct? within perf budget? SRP seams held? no `entt::` leak into Lua/render? sandbox not widened?
docs updated (roadmap status, any new design note)? Then commit with a clear message (what + why +
before/after perf if relevant) and, if the user asked, push.

## Performance philosophy
Instrumented from day one — measure, don't guess. Every hot path is timed (`Metrics::Scope`), gated
(`bench/`), and profilable (`ADVENTURE_PROFILE`). A feature isn't done until it's proven not to regress the
frame budget. Keep the headroom; we render retro-cheap on purpose.
