# Backlog — captured feature requests

Requested features not yet scheduled into an active milestone. Each lands via the `adv-feature` loop when
its milestone comes up. Keep this lean; promote items into `ROADMAP.md` when work starts.

## Chests, loot & keys (→ M3 inventory, + crafting)
Containers that hold items/loot. Some are **locked** and need a **key**. Keys are either **found** in the
world or **crafted from raw materials**.
- Needs first: item/inventory system (M3), world interaction ("use" a chest), a lock/key data model.
- Design sketch: a `Container` (inventory + optional `lockId`); a `Key` item with a matching `lockId`;
  opening checks the player holds a matching key (or a lockpick skill later). Loot tables (Lua-driven) fill
  chests. Crafting: a recipe (raw materials → key/item) — a small data-driven crafting system (M3/M5).
- Data-driven in Lua: chest contents, loot tables, recipes, lock/key ids.

## Dark Messiah–style skill / talent points (→ M4 RPG stats)
A lightweight talent system modelled on *Dark Messiah of Might & Magic*. Distilled from the DM reference:

**Earning points** — from **quests** and **discovering hidden locations** (not XP grinding). This directly
rewards our exploration/secrets pillar. Spend points to unlock/upgrade nodes; some nodes **gate equipment**.

**Three trees** (numbers are DM's, as a starting balance reference):
- **Combat** — *Close Combat* L1/L2/L3 (1/2/4 pts: combos+charge → shield use + disarm → jump/spin/defense-
  break); *Archery* L1/L2/L3 (1/2/4: zoom → steady aim → fire rate); *Mighty Power* (6/8/10 → +2/3/6 weapon
  damage, **gates mid/late weapons**); *Critical* (6/8 → +2/4% double-dmg); *Adrenaline* (12 → two adrenaline
  bars, empowered attacks).
- **Magic** (a spell = two-handed, cancels shield; except Darkvision/Heal). *Attack*: Flame Arrow(1),
  Flame Trap(2), Freeze(3), Fireball(7), Lightning(7), Hellfire(10). *Utility*: **Telekinesis(1)** (gravity-
  gun — pull/throw objects; **ties to our physics/kick environmental kills** + reaching secrets), Heal(3),
  Charm(3), Sanctuary(7, invuln), Weaken(10). Darkvision = free vision mode.
- **Other** — *Body*: Endurance(1, stamina), Health(4/7/10 → 60/70/80, **gates warrior armor**), Poison
  Resist(6), Stamina Regen(12). *Mind/Tech*: **Navigation(1)** (idle → secret switches/doors glow; **ties to
  hidden areas**), Mana Affinity(2/5/10 → 40/70/100, **gates mage gear**), Mana Regen(12), *Stealth* L1/L2/L3
  (1/4/10: quiet+pickpocket → backstab → full hide; **gates thief gear**), **Lockpicking(8)** (open locked
  doors/chests, reveal traps; **ties to the locked-chest/key feature above**).

**Weapons/gear as data** (fits the `WeaponDef` Lua plan): each item = attack power + skill requirement +
optional special (double-damage vs a faction/type, elemental status like freeze/shock/burn). Classes: daggers
(fast, backstab), swords (best all-round, shield-compatible), bows (stealth ranged), shields (block ranged,
require Combat skill), staves (slow AoE stun, no shield), armor (defense + skill gate), rings (one slot).

**Our lightweight cut**: keep the tree shape + gating + quest/secret point income, trim the node count. All
trees/nodes/costs/effects and weapon defs live in **Lua**; C++ holds earned/spent state + applies effects.

## Notes
- These reinforce the existing plan (M3 inventory, M4 stats, M5 spells) rather than adding new pillars.
- When promoting an item, add its acceptance criteria + tests up front (per `adv-feature`).
