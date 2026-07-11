---
name: adv-input-action
description: Add or change a player input action (a key/mouse-driven control) the right way — routed through the rebindable input action map, never a hard-coded key. Use whenever you introduce a new control (a new key/button the player presses) or change how an existing control is read, so it stays rebindable via keybindings.cfg.
---

# adv-input-action — wire a control through the action map

**Rule: no hard-coded gameplay keys.** Any control the player presses goes through `src/input`
(`Action` + `actionDown/Pressed/Released`), so it is rebindable via `keybindings.cfg`. `IsKeyDown(KEY_*)` /
`IsMouseButton*` are only allowed for **dev tools** (F3–F6, V, B) — never for gameplay.

## When to use
- Adding a new control (weapon swap, dodge, sprint-toggle, inventory, …).
- Changing how an existing control is polled.
- A review/CI catches a raw `IsKeyDown`/`IsMouseButton*` in gameplay code.

## Checklist — adding a new action `Foo`
1. **Enum** — add `Foo` to `enum class Action` in `src/input/InputMap.h`, **before `Count`**
   (`kActionCount` derives from `Count`).
2. **Name** — add `"Foo"` to `kNames[]` in `src/input/InputMap.cpp`, in the **same order** as the enum
   (the name is the stable id in `keybindings.cfg`).
3. **Default** — set `m.codes[(int)Action::Foo] = <code>;` in `defaultBindings()`. A key is its raylib code
   (add a `K_*` local if needed, to keep this TU raylib-free); a mouse button is `mouseCode(button)`.
4. **Query in `main`** — read it with `actionPressed/Down/Released(keys, Action::Foo)` where the control is
   handled. Never `IsKeyDown(KEY_*)` for gameplay.
5. **Build + test** — `adv-test`. `test_inputmap.cpp` ("default bindings cover every action") fails if you
   forgot the default; add a case if the action has interesting parse/round-trip behaviour.
6. **Docs** — update `src/input/CLAUDE.md` (and the controls line in root `CLAUDE.md`) if the control is
   player-facing.

No config migration needed: `main` **rewrites `keybindings.cfg` on launch** (merges defaults over the
player's overrides), so a new action appears in the file automatically while existing binds are kept.

## Gotchas
- Enum order **must** match `kNames[]` order — they index the same array. A mismatch mislabels the config.
- Codes: letters are ASCII (`'W'`=87); mouse is `kMouseBase(1000)+button` so it never collides with a key.
- Keep `InputMap` pure (no raylib include). Live polling belongs in `InputQuery` (raylib).
- Charge/hold controls need all three: `Pressed` (start), `Down` (hold), `Released` (fire) — see Attack.

## Files
`src/input/InputMap.{h,cpp}` (pure map) · `src/input/InputQuery.{h,cpp}` (raylib polling) ·
`tests/test_inputmap.cpp` · gameplay wiring in `src/main.cpp` · `keybindings.cfg` (generated next to the exe).
