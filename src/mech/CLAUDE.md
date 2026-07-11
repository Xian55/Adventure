# src/mech — CLAUDE.md

Level mechanisms: doors + activators (levers, pressure plates), wired Quake-style by `target`/`targetname`.
Pure logic (no raylib window) -> headless-testable. Drawn by `render/Mech`.

| File | Responsibility |
|------|----------------|
| `Mechanisms.{h,cpp}` | `Door` (sliding solid box, animated `t` 0→1), `Lever` (E-toggle → fires `target`), `Plate` (weight sensor → fires `target`). `updateDoors` (ease open/closed), `updatePlates` (pressed if any occupant point is on the pad), `applyActivations` (a door's `wantOpen` = manual OR any linked lever/plate — unless still locked), `useDoor` (a locked door spends a key; a plain door toggles), `doorSolid`/`doorCenter` (collision + render position), `nearestLever`/`nearestDoor` (E prompts). |

## Wiring
- An activator's `target` string matches a door's `targetname`. A door opens while **any** linked lever is on
  or plate is pressed. Doors can also be **key-locked** (`lockId` > 0 → E + a key opens) or a plain
  **E-toggle** door (no `targetname`).
- Map entities: `func_door` (`targetname`/`lock`/`size` "w d h" map units/`move` open translation),
  `lever` (`target`), `func_plate` (`target`/`size` — **height is the last size value**; map-Z is up).
- `main`: E-use priority is lever → door → chest. Each fixed step builds plate `occupants` from player +
  enemy + prop feet, runs `updatePlates` → `applyActivations` → `updateDoors`. Doors are solid via
  `doorSolid` fed into `items::collideActorBoxes` (player). Enemy-vs-door collision is not wired yet.

## Rules / gotchas
- Keep it pure/tested (`tests/test_mechanisms.cpp`). Numbers (open time, sizes) are map/struct data.
- **Size axis swap**: map dims `(x, y, z)` → engine half `(x, z, y) * kMapScale / 2` (map-Z = engine-up).
  A flat plate has its **small** dim last (`"64 64 8"`); a tall door has it in the middle→up (`"64 12 96"`).

## Coming
Enemy-vs-door collision; door open/close SFX; timed / one-shot doors; `trigger_multiple`/`func_*` → Lua
events (M6); movable objects the player can carry and drop onto a plate.
