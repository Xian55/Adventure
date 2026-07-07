# src/combat — CLAUDE.md

Combat rules. Pure logic (no raylib) -> headless-testable. Numbers live in Lua; execution in C++.

| File | Responsibility |
|------|----------------|
| `Melee.{h,cpp}` | Dark Messiah–style directional melee: **hold to wind up (Charge), release to strike**. `beginCharge`/`setSwingDir`/`releaseSwing` + `updateMelee` (Idle→Charge→Active→Recovery→Idle). `SwingDir` from WASD (Left/Right/Forward/Overhead); **Neutral (no key) alternates Left/Right**. `chargeFraction` scales damage (hold longer = stronger). `WeaponDef` from Lua. Same charge/release pattern a **bow draw** will reuse (M5). |
| `Enemy.h` | `Enemy` struct (pos/vel/health/`EnemyState` Approach/Stagger/Dead/timer/dims) — a skeleton. Rendered as a placeholder box until sprites. |
| `CombatSystem.{h,cpp}` | `updateEnemies` (approach AI + knockback integration + stagger/death timers) and `resolveMeleeHits` (Active hitbox vs enemies in reach+arc → damage/knockback/stagger/kill, one hit per swing). `EnemyTuning`. |

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
