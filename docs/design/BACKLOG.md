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
A lightweight talent system inspired by *Dark Messiah of Might & Magic*: earn **skill points** at
progression milestones and spend them across a few trees.
- Sketch (to refine against the DM reference — the shared namu.wiki page 403'd, paste specifics to pin down):
  three small trees — **Combat** (weapon mastery, power attacks, health/stamina, block/parry),
  **Magic** (unlock/upgrade spells: fire, lightning, freeze, telekinesis, heal), **Stealth/Utility**
  (assassination, lockpicking → ties into chest keys, agility). Modest node count; meaningful choices
  (can't max everything). Tiered prerequisites; per-node caps.
- Data-driven: define trees/nodes/costs in Lua; C++ holds the earned/spent state + applies stat effects.
- Ties in: lockpicking node ↔ locked chests above; weapon mastery ↔ M2 melee; spells ↔ M5.

## Notes
- These reinforce the existing plan (M3 inventory, M4 stats, M5 spells) rather than adding new pillars.
- When promoting an item, add its acceptance criteria + tests up front (per `adv-feature`).
