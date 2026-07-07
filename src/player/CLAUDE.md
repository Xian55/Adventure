# src/player — CLAUDE.md

First-person player: state + movement/collision. Pure logic (no raylib window) -> headless-testable.

| File | Responsibility |
|------|----------------|
| `Player.h` | State struct: `position` (AABB center, engine space), `velocity`, `yaw`/`pitch`, `health`/`maxHealth`, `onGround`, `crouched`. |
| `MoveTuning.h` | Feel constants (speed/accel/friction/gravity/jump) loaded from Lua `tuning`; dims (radius/height/eye) stay in C++. |
| `PlayerController.{h,cpp}` | `updatePlayer()` — one fixed-step: Quake accelerate/friction + axis-separated swept-AABB collision vs `world::CollisionWorld`. `MoveInput` = forward/right/jump/crouch/sprint. |
| `JumpMeter.{h,cpp}` | Records jumps for tuning/level design: detects takeoff/landing, reports last jump (distance/height/airtime) + session maxima. Pure -> tested. Feeds the F4 telemetry HUD. |

## How it works
- **Quake accel**: ground friction, then `accelerate(wishdir, speed, accel)`; air uses a low `airAccel`
  (air-strafe/bhop emerges). Gravity each step; jump sets `vel.y` when `onGround`.
- **Collision**: integrate X, then Z (walls slide), then Y; each axis reverts + zeroes its velocity if the
  new center overlaps a solid. `onGround` set when a downward Y move is blocked.
- **Stair step-up**: a blocked grounded move retries lifted by `stepHeight` (0.5) and settles onto the step,
  so you walk stairs/ledges ≤ stepHeight instead of jumping; taller ledges still block. (`updatePlayer`.)
- **Crouch**: shifts the center to keep feet planted; standing up needs headroom (inset test box so
  floor-contact isn't a false block).
- Fixed timestep (`kFixedDt`, 60 Hz) — run in `main`'s accumulator loop. Mouse-look updates `yaw`/`pitch`
  per display frame (smooth); the camera is built from the player each frame.

## Rules / gotchas
- Movement is deterministic given input + tuning (no wall-clock) — that's what makes it testable and
  keeps the door open for co-op (see the determinism note in ARCHITECTURE).
- Feel is tuned in `scripts/tuning.lua` (hot-reload), read via `sScript->evalNumber("tuning.x", def)`.
- `position` is the AABB **center**; feet = `position.y - height/2`, eye = feet + `eyeHeight`.

## Coming
Step-up assist (stairs), crouch head-room re-expand check, kick impulse (M2), then move into the ECS as a
`PlayerController` component/system at M2.
