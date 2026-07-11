# src/items — CLAUDE.md

Items, inventory, and world pickups. Pure logic (no raylib window) -> headless-testable. Gameplay data,
not presentation (pickups are *drawn* by `render/Prop`).

| File | Responsibility |
|------|----------------|
| `Item.{h,cpp}` | The item **set** + kinds. `ItemKind` (Consumable/Coin/Key/Weapon), `ItemId` constants, `ItemDef` (name/kind/maxStack/value), `itemDef(id)` lookup. The definitions are a C++ table **for now** — moving them to Lua needs a table-read binding (see Coming). |
| `Inventory.{h,cpp}` | Slot-based bag: `ItemStack`s up to each item's `maxStack`, bounded by `capacity`. `addItem` (tops up existing stacks, then opens slots; returns amount taken), `removeItem`, `itemCount`. Pure/tested. |
| `Pickup.{h,cpp}` | `Pickup` = a floating item in the world (`itemId` + position). `collectPickups` = player over one → a **Consumable is used on the spot** (heal, clamped); anything else goes to the `Inventory` (stays if full). Pure/tested. |

## Flow
- Destructibles (`combat/Destructible`) carry a `dropItem` id; breaking one spawns a `Pickup`. The map's
  `loot` key on a `prop_*` entity picks the item (`health`/`coin`/`key`); a keg defaults to a coin.
- `main` runs `collectPickups` each fixed step; the HUD reads `itemCount` for the coin/key/potion line.
- `render/Prop::drawPickups` colours the orb by `itemDef(itemId).kind` (green consumable / gold coin / steel key).

## Rules / gotchas
- Item **definitions** live here (C++); tunable numbers will move to Lua when the bridge can read a table.
- Keep this pure — no raylib window calls. Consumable effects (heal) are applied by value, not by drawing.
- `itemDef` returns the None def for unknown ids (never throws / null).

## Coming
- **Lua item defs**: iterate an `items` table from `scripts/` (needs a `ScriptEngine` table/string read beyond
  `evalNumber`). Then item stats/loot tables are data.
- Equip / hotbar (weapons as items → `WeaponDef`), a Lua-driven HUD, chests + locks/keys (`Container` +
  `lockId`), loot tables (M3 backlog).
