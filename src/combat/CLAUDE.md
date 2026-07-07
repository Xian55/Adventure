# src/combat — CLAUDE.md

Combat rules. Pure logic (no raylib) -> headless-testable. Numbers live in Lua; execution in C++.

| File | Responsibility |
|------|----------------|
| `Melee.{h,cpp}` | Dark Messiah–style directional melee: **hold to wind up (Charge), release to strike**. `beginCharge`/`setSwingDir`/`releaseSwing` + `updateMelee` (Idle→Charge→Active→Recovery→Idle). `SwingDir` from WASD (Left/Right/Forward/Overhead); **Neutral (no key) alternates Left/Right**. Direction **snapshots at the first key picked during the charge** (`dirLocked`) — later key changes are ignored, so a swing commits to what you aimed. `chargeFraction` scales damage (hold longer = stronger). `WeaponDef` from Lua. Same charge/release pattern a **bow draw** will reuse (M5). |
| `Enemy.h` | `Enemy` struct (pos/vel/health/`EnemyState` Approach/**Windup**/**Recover**/Stagger/Dead/timer/dims) — a skeleton. Rendered as a placeholder box until sprites. |
| `CombatSystem.{h,cpp}` | `updateEnemies` (approach AI → **Windup telegraph → strike the player** (reduced by a facing shield) → Recover; knockback integration; stagger/death timers; writes `PlayerTarget.health`), `resolveMeleeHits` (Active hitbox vs enemies in reach+arc → damage/knockback/stagger/kill, one hit per swing), `tryKick` (shove enemies in a forward cone hard + stagger — environmental-kill move; also interrupts a windup). `EnemyTuning` (approach/attack/block numbers), `PlayerTarget` (what an enemy needs to hit back). |

## Flow
- Input buffers a swing (`requestSwing`, on attack press). `updateMelee` runs each **fixed step** (in
  `main`'s accumulator loop). The sword viewmodel animates from `(phase, phaseProgress)`.
- The **hitbox is live only during Active**; its center sweeps forward across `arc` over the Active window.
  `hitThisSwing` debounces so one swing hits each target once (used by M2b hit resolution).
- Weapon numbers come from `scripts/weapons/sword.lua` via `evalNumber` (hot-reload with F5).

## Rules
- Keep the state machine pure/tested (`tests/test_melee.cpp`) — feel is tuned in Lua, not code.
- Execution (hitbox geometry, damage application) is C++; policy/numbers are Lua.

## Shield & the hit-back loop (M2c)
- Enemies **strike back**: Approach → **Windup** (committed telegraph, immobile — dodge or kick to cancel) →
  strike (damage if the player is still within `attackReach`) → **Recover** → Approach.
- **Shield** = hold RMB. A raised shield facing the attacker (enemy inside the `blockArc` front cone)
  absorbs `blockReduction` of the hit; flank/back hits land full. State lives in `main`; passed in via
  `PlayerTarget.shieldRaised` (kept out of the pure logic's own state — it's per-frame input).
- Player **health** on `Player`; `updateEnemies` writes it through `PlayerTarget.health*`. Death (≤0) →
  `main` respawns via `loadMap` (resets map, enemies, health). HUD draws a health bar + BLOCK indicator.
- Enemy attack numbers are Lua-tunable (`tuning.enemy*`, `tuning.block*`), hot-reloaded with F5.

## Coming
Rage → berserk meter (repeated hits build rage; maxed = temporary berserk — DM resource). Swap enemy boxes
→ billboard sprites (`RenderKind` seam). Player + enemies move into the ECS as components.
