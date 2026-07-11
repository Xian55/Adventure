# src/input — CLAUDE.md

Rebindable input: named **actions** mapped to input codes, so keys aren't hard-coded. The map is pure
(headless-testable); live polling is a thin raylib layer.

| File | Responsibility |
|------|----------------|
| `InputMap.{h,cpp}` | `Action` enum (MoveForward/Back/Left/Right, Jump, Crouch, Sprint, Attack, Block, Kick, **Interact**) → per-action **code**. A code is a raylib key code, or a mouse button encoded as `kMouseBase(1000)+button`. `defaultBindings`, `bindAction`, `actionCode`, `actionName` (config id), `codeName` (UI label), `saveBindings`/`loadBindings` (text config). **Pure/tested.** |
| `InputQuery.{h,cpp}` | `actionDown`/`actionPressed`/`actionReleased` — dispatch a bound code to raylib's key/mouse polling. Needs a window, so it's kept out of the pure map. |

## Flow
- `main` loads `keybindings.cfg` (next to the exe) at startup if present, else writes the default template
  (so there's a file to edit). No in-game rebind UI — the file **is** the interface.
- All gameplay input goes through `actionDown/Pressed/Released(keys, Action::X)`. Dev keys (F3–F6, V, B)
  stay hard-coded on purpose.
- Config format: one `Action code` per line; `#` comments and junk/unknown lines are ignored; missing
  actions keep their defaults (forward-compatible).

## Rules / gotchas
- Keep `InputMap` pure (no raylib) — it holds default key codes as local enums to avoid the raylib include.
- Codes: letters are their ASCII (`'W'`=87); mouse is `1000+button` (never collides with key codes).
- New action? Use the **`adv-input-action`** skill — add to `Action` (before `Count`), `kNames`, and
  `defaultBindings` (all same order), then wire its query in `main`. `keybindings.cfg` self-updates on launch.

## Coming
Mouse sensitivity / invert-Y as bindable settings; optional in-game rebind screen; controller support.
