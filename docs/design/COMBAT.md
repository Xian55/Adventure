# Combat Design

North star: **Dark Messiah of Might & Magic** — physical, reactive melee where the environment is a weapon.
Weight and commitment over button-mashing. Every swing is a small state machine; every hit has knockback;
the **kick** turns the world's hazards (spikes, ledges, pits) into your best damage source.

## Melee
Each weapon is a state machine: **Idle → Windup → Active → Recovery**, with per-phase durations from the
weapon's Lua def. During **Active** a hitbox sweeps forward across the weapon's arc (sub-sampled so fast
frames don't leave gaps); a per-swing hit set means one hit per enemy per swing. A combo window in Recovery
advances the combo step.

Weapon families (each with pros/cons, tuned in Lua):
- **Daggers** — fast, short, low damage, quick recovery; backstab bonus.
- **Swords** — balanced reach/speed/damage.
- **Maces** — slow, high stagger/knockback, good vs shields/armour.
- **Axes** — high damage, slower, strong on wind-up hits.
- **Staves** — reach + spell channelling; weaker raw melee.

## Kick (signature)
A short, cooldown-gated forward impulse. Casts a short AABB/sphere (~1.6 u) along camera-forward; on hit,
applies a large horizontal knockback to the enemy's velocity, integrated by the same physics as everything
else — so kicking an enemy into a `trigger_hurt` brush or off a ledge is an *emergent* environmental kill,
not a scripted special case. Reach/impulse live in `tuning.lua` (`kickReach`/`kickImpulse`). A kick also
**interrupts** an enemy mid-windup (it staggers), cancelling the pending strike. *(Implemented — key F.)*

## Shield / block
Facing-based. While raised (**hold RMB**), incoming damage is reduced by `blockReduction` **iff** the
attacker is within the shield's `blockArc` of the player's facing (`dot(forward, dirToEnemy) > cos(arc)`).
Flank/back hits ignore the shield. *(Implemented: damage reduction + HUD BLOCK indicator.)* **Planned:** a
tight **parry** window that staggers the attacker, a move-speed penalty while raised, and guard-break on
heavy/mace hits.

## Enemy behaviour (skeleton = first)
State machine with C++ transitions, **Lua policy**: **Approach** (steer toward player) → **Windup**
(committed telegraph, immobile — the player's dodge/kick window) → strike (damage iff still in
`attackReach`) → **Recover** → **Stagger** (locked while stagger timer > 0; knockback plays; a hit or kick
forces this, interrupting a windup) → **Dead** (shrink, despawn; Lua `OnDeath` may drop loot later).
*(Implemented in `updateEnemies`; numbers in `tuning.enemy*`. Lua per-enemy policy hooks come with the ECS
move.)* *Decisions* (when
to attack, aggro range, cadence) are Lua `OnThink(self, dt)`; *execution* (movement integration, hitbox
geometry, collision) is C++.

## The C++ / Lua split
- **C++ (hot path):** state-machine ticking, hitbox/hurtbox sweep + detection, damage/knockback/stagger
  application, shield facing math, kick hit test, physics integration.
- **Lua (data + policy, hot-reload):** weapon numbers/timings, enemy AI policy, arena spawn/win, all tuning.

## Feel tuning
Every movement/combat constant lives in a hot-reloadable Lua `tuning` table (`scripts/tuning.lua`). Iterate
feel by editing Lua and reloading, not by rebuilding. A debug overlay (F3 metrics now; hitbox/hurtbox
wireframes in M2) makes the invisible visible.
