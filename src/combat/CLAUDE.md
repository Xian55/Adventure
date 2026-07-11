# src/combat — CLAUDE.md

Combat rules. Pure logic (no raylib) -> headless-testable. Numbers live in Lua; execution in C++.

| File | Responsibility |
|------|----------------|
| `Melee.{h,cpp}` | Dark Messiah–style directional melee: **hold to wind up (Charge), release to strike**. `beginCharge`/`setSwingDir`/`releaseSwing` + `updateMelee` (Idle→Charge→Active→Recovery→Idle). `SwingDir` from WASD (Left/Right/Forward/Overhead); **Neutral (no key) alternates Left/Right**. Direction **snapshots at the first key picked during the charge** (`dirLocked`) — later key changes are ignored, so a swing commits to what you aimed. `chargeFraction` scales damage (hold longer = stronger). `WeaponDef` from Lua. Same charge/release pattern a **bow draw** will reuse (M5). |
| `Enemy.h` | `Enemy` struct (pos/vel/health/`EnemyState` Approach/**Windup**/**Recover**/Stagger/Dead/timer/dims + `RenderKind`) — a skeleton. Drawn as a Y-facing **billboard** sprite (`render/Billboard`); `RenderKind::Box` is the debug cube (toggle **B**). |
| `CombatSystem.{h,cpp}` | `updateEnemies` (approach AI → **Windup telegraph → strike the player** (reduced by a facing shield) → Recover; knockback integration; stagger/death timers; writes `PlayerTarget.health`), `resolveMeleeHits` (Active hitbox vs enemies in reach+arc → damage×`damageMul`/knockback/stagger/kill, one hit per swing; returns `MeleeHitResult{hits,kills}` to feed rage), `tryKick` (shove enemies in a forward cone hard + stagger — environmental-kill move; also interrupts a windup), `applyHazards` (damage enemies whose feet are in a `world::Hazard` volume → kick them into lava for the kill). `EnemyTuning` (approach/attack/block numbers), `PlayerTarget` (what an enemy needs to hit back). |
| `Rage.{h,cpp}` | Rage → berserk meter (DM combat resource). `addRage` (landed melee/kills build it; maxing flips to berserk), `updateRage` (drain over the berserk window, else bleed after a grace delay), `rageDamageMul`/`rageSpeedMul` (berserk buffs), `rageFraction` (HUD). Pure/tested. `RageTuning` from Lua. |
| `Spell.{h,cpp}` | Spells + mana. `SpellDef` (from `scripts/spells.lua`), `Mana` + `updateMana` (regen to cap), `castPush` (Telekinesis — spend mana, shove enemies in a forward cone within range into their velocity + stagger → the ranged kick, into hazards). Pure/tested. `main` binds it to `Action::Cast` (R) with a HUD mana bar. |
| `Projectile.{h,cpp}` | Crossbow bolts (later spells). `updateProjectiles` (fly + slight gravity + lifetime; a `stuck` bolt stops but ages out), `resolveProjectileHits` (sphere-test the first enemy each bolt hits → damage/stagger/kill, spend the bolt). Pure/tested. `main` fires on release when `WeaponDef.ranged`, sticks bolts to walls via `CollisionWorld`, and renders them along `dir`. |
| `Loadout.{h,cpp}` | Equip / weapon-swap. `weaponDefFor(itemId, swordDef)` seeds the Dagger/Mace presets (then `main` overrides each from `scripts/weapons/dagger.lua`/`mace.lua`; the Sword loads from `sword.lua`), `nextWeapon(inv, current)` (cycle the weapons the player holds), `isWeapon`/`weaponName`. Pure/tested. `main` binds the cycle to `Action::NextWeapon` + an `equipDef` picker. |
| `Destructible.{h,cpp}` | Smashable props (`PropKind` Barrel/Crate/Keg) with health + optional `LootKind`. `damageProps` (kick/blast in a radius → break → spawn a `Pickup`), `updateProps` (debris timer → despawn), `collectPickups` (player over a pickup heals, clamped), `resolveActorProps` (push an actor cylinder out of intact props → props block movement). Pure/tested. Drawn by `render/Prop`. |

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

## Rage → berserk (M2d)
`Rage.{h,cpp}`, pure/tested. Landed melee builds rage (`gainPerHit`, kills add `gainPerKill`); filling the
meter flips to **berserk** for `berserkDuration` — melee damage ×`damageMul`, swings ×`speedMul` (main scales
`updateMelee`'s dt) — then the meter drains to empty and resets. Idle bleeds rage after `decayDelay`. Death
clears it. `resolveMeleeHits` returns the hit/kill count that feeds `addRage`; berserk multipliers flow back
into `resolveMeleeHits(..., damageMul)` and the swing-speed dt. Numbers in `tuning.rage*`/`tuning.berserk*`.

## Destructibles & loot (M2)
`Destructible.{h,cpp}` + `render/Prop`. Props spawn from `prop_barrel`/`prop_crate`/`prop_keg` map entities
(ground-snapped like enemies). **Melee smashes them inside `resolveMeleeHits`** — the same swing that hits
enemies damages props in its arc, sharing the one-per-swing `hitThisSwing` debounce. (Earlier the prop hit was
gated on a Charge→Active transition detected in the fixed loop, but that transition happens in `releaseSwing`
in the input pass, so the sword never damaged props — fixed by folding it into `resolveMeleeHits`.) Kick
shatters outright via `damageProps`. A prop's `dropItem` (from the map `loot` key: health/coin/key; keg → coin) spawns an
`items::Pickup` on break — consumables heal on touch, coins/keys go to the inventory (see `src/items`).
Intact props **block** the player and enemies (`resolveActorProps`,
run after `updatePlayer`/`updateEnemies`); broken rubble is walk-through. `PropTuning` = C++ defaults for now.

## Coming
Player + enemies move into the ECS as components. Prop-collision broadphase when prop counts grow (linear
now). More loot kinds + a real inventory (M3). Real sprite/model art.
