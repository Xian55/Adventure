# src/items — CLAUDE.md

Items, inventory, and world pickups. Pure logic (no raylib window) -> headless-testable. Gameplay data,
not presentation (pickups are *drawn* by `render/Prop`).

| File | Responsibility |
|------|----------------|
| `Item.{h,cpp}` | The item **set** + kinds. `ItemKind` (Consumable/Coin/Key/Weapon), `ItemId` constants, `ItemDef` (name/kind/maxStack/value), `itemDef(id)` lookup, `setItemDef(id,name,maxStack,value)`. Numbers + names come from **`scripts/items.lua`** (loaded by `main` via `evalString`/`evalNumber`, hot-reload F5); the id + kind stay in C++ (code branches on kind). |
| `Inventory.{h,cpp}` | Slot-based bag: `ItemStack`s up to each item's `maxStack`, bounded by `capacity`. `addItem` (tops up existing stacks, then opens slots; returns amount taken), `removeItem`, `itemCount`. Pure/tested. |
| `Pickup.{h,cpp}` | `Pickup` = a floating item in the world (`itemId` + `count` + position). `collectPickups` = player over one → a **Consumable is used on the spot** (heal × count, clamped); anything else goes to the `Inventory` (a full bag leaves the remainder). Pure/tested. |
| `Container.{h,cpp}` | Chests: `Container` (contents + optional lock). `tryOpenContainer` (locked → spends a key from the inventory; opening spills contents as pickups), `nearestContainer` (closest facing chest in range for the "use" prompt), `resolveActorContainers` (chests block enemies horizontally). Pure/tested. |
| `Collide.{h,cpp}` | `SolidBox` + `collideActorBoxes`: min-penetration AABB resolve so props/chests block from the sides **and** can be stood on (land on top → grounded + zero downward vel; head-bonk from below). Used for the player. Pure/tested. |

## Flow
- Destructibles (`combat/Destructible`) carry a `dropItem` id; breaking one spawns a `Pickup`. The map's
  `loot` key on a `prop_*` entity picks the item (`health`/`coin`/`key`); a keg defaults to a coin.
- Chests spawn from `prop_chest` entities (`coins`/`potions`/`keys` count keys → contents; `lock` > 0 =
  locked). Face one + press **E** → `tryOpenContainer` (a locked one spends a key). The loop:
  barrel → key → locked chest → loot spills → collect.
- `main` runs `collectPickups` each fixed step; the HUD reads `itemCount` for the coin/key/potion line and
  shows the chest "use" prompt. Props + chests are solid via `collideActorBoxes` (player) /
  `resolveActorContainers`+`resolveActorProps` (enemies, horizontal only — they don't jump).
- `render/Prop`: `drawPickups` colours the orb by item kind; `drawContainers` draws the chest (hinged lid,
  lock plate).

## Rules / gotchas
- Item **definitions** live here (C++); tunable numbers will move to Lua when the bridge can read a table.
- Keep this pure — no raylib window calls. Consumable effects (heal) are applied by value, not by drawing.
- `itemDef` returns the None def for unknown ids (never throws / null).

## Coming
- **Lua item defs**: iterate an `items` table from `scripts/` (needs a `ScriptEngine` table/string read beyond
  `evalNumber`). Then item stats/loot tables are data.
- Equip / hotbar (weapons as items → `WeaponDef`), a Lua-driven HUD, chests + locks/keys (`Container` +
  `lockId`), loot tables (M3 backlog).
