# src/combat — CLAUDE.md

Combat rules. Pure logic (no raylib) -> headless-testable. Numbers live in Lua; execution in C++.

| File | Responsibility |
|------|----------------|
| `Melee.{h,cpp}` | Melee swing state machine: `WeaponDef` (timings/reach/arc/damage/knockback, from Lua), `MeleeState` (phase/timer/combo/flags), `updateMelee` (Idle→Windup→Active→Recovery→Idle + Recovery→Windup combo), `hitboxActive` (Active only), `phaseProgress` (0..1 for the sweep + viewmodel). |

## Flow
- Input buffers a swing (`requestSwing`, on attack press). `updateMelee` runs each **fixed step** (in
  `main`'s accumulator loop). The sword viewmodel animates from `(phase, phaseProgress)`.
- The **hitbox is live only during Active**; its center sweeps forward across `arc` over the Active window.
  `hitThisSwing` debounces so one swing hits each target once (used by M2b hit resolution).
- Weapon numbers come from `scripts/weapons/sword.lua` via `evalNumber` (hot-reload with F5).

## Rules
- Keep the state machine pure/tested (`tests/test_melee.cpp`) — feel is tuned in Lua, not code.
- Execution (hitbox geometry, damage application) is C++; policy/numbers are Lua.

## Coming (M2b/M2c)
Hit resolution (Active hitbox vs enemy hurtbox → damage/knockback/stagger), skeleton `EnemyAI`
(approach/attack/stagger/die), kick (forward impulse) + shield (facing-based block). Player moves into the
ECS as components at that point.
