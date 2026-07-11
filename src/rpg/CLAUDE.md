# src/rpg — CLAUDE.md

RPG progression: skill trees → derived stats. Pure logic (no raylib) -> headless-testable. A lightweight cut
of the Dark Messiah trees (see `docs/design/BACKLOG.md`): keep the shape (trees, ranks, prereqs, gating),
trim the node count.

| File | Responsibility |
|------|----------------|
| `SkillTree.{h,cpp}` | `SkillId` + a fixed `SkillNode` table (name/tree/maxRank/per-rank cost/prereq). `SkillState` (points + rank per node). `skillCost`, `canUnlock` (points + prereq + not maxed), `unlockSkill` (spend + rank up). `deriveStats(state)` → `Stats` (maxHealthBonus, damageMul, moveSpeedMul, rageBuildMul, lockpick). Pure/tested. |

## Nodes (current)
- **Toughness** (Combat ×3) → +20 max HP/rank · **Power** (Combat ×2, needs Toughness) → +15% dmg/rank ·
  **Adrenaline** (Combat, needs Power) → rage ×1.5 · **Endurance** (Body ×2) → +8% move speed/rank ·
  **Lockpicking** (Utility) → open locked chests/doors without a key.

## Flow (in `main`)
- `SkillState skills` **persists across respawn** (not reset in `loadMap`). Each fixed step derives `Stats`
  and applies them: `maxHealth = 100 + maxHealthBonus`, melee `damageMul`, a scaled `MoveTuning` copy for
  move speed, a scaled `RageTuning` copy for rage build, `lockpick` passed to `tryOpenContainer`/`useDoor`.
- **Point income**: a kill grants 1 point (`Enemy.scored` debounces). **Placeholder** — the DM design earns
  points from **quests + hidden locations** (M6/M7), not kill-grinding; swap the income source then.
- **Spend UI**: the `SkillMenu` action (default K, rebindable via `src/input`) opens a native-res overlay
  that pauses the sim; Up/Down select, Enter unlocks. (The only in-game menu so far.)

## Rules / gotchas
- Keep it pure/tested; numbers live in the `kNodes` table (move to Lua when the bridge can read a table).
- Effects are derived, not stored — recompute `deriveStats` rather than caching mutable stat fields.
- New node? extend `SkillId` (before `Count`), add its `kNodes` row + its effect in `deriveStats`, add a test.

## Coming
Stamina + attributes; more trees (Magic → spells at M5); real point income (quests/secrets); Lua-defined
nodes; gear/skill gating on weapons (`WeaponDef` requirement).
