---
name: adv-map
description: Authoritative reference for the TrenchBroom/Quake Valve 220 .map format and Adventure's map conventions. Use when authoring, generating, editing, debugging, or validating a .map file, or wiring TrenchBroom to the project. TrenchBroom has no MCP server or CLI/scripting API — .map is plain text, so we generate/validate it ourselves (parser: src/world/MapParser).
---

# adv-map — the .map format authority

**TrenchBroom has no MCP server and no CLI/scripting API.** It is a GUI editor. But `.map` is **plain
text**, so we author interactively in TrenchBroom *and* generate/transform/validate maps programmatically.
Our engine parses Valve 220 (`src/world/MapParser`) and builds convex geometry + collision
(`src/world/BrushGeometry`).

## Valve 220 grammar
```
map     := entity*
entity  := '{' (keyvalue | brush)* '}'
keyvalue:= '"' key '"' '"' value '"'
brush   := '{' face+ '}'          // >= 4 faces (a convex solid)
face    := '(' x y z ')' '(' x y z ')' '(' x y z ')'   // 3 points => a plane
           TEXTURE
           '[' ux uy uz uoffset ']'   // Valve 220 U texture axis + offset
           '[' vx vy vz voffset ']'   // V texture axis + offset
           rotation xscale yscale
```
- A **brush** is the convex intersection of its faces' half-spaces. Three points define each face's plane.
- **Point entities** (no brushes) carry `origin` `"x y z"`, `angle`, `classname`, etc.
- Comments: `//` to end of line. First two lines are conventionally `// Game:` / `// Format: Valve`.

## Adventure conventions (how our loader reads it)
- **Winding-agnostic**: our `BrushGeometry` orients each face normal outward from the brush interior, so
  the 3 points may be in **any order** — handy when hand-generating brushes.
- **Coordinates**: map is **Z-up**; engine is **Y-up**. Conversion (one place, `BrushGeometry::toEngine`):
  `engine = (x, z, -y) * kMapScale`, with `kMapScale = 1/32` (32 map units = 1 engine unit).
- **Texture name = loose PNG name** in `assets/textures/<name>.png` (see `adv-docs`/ASSET_PIPELINE).
  `clip`/`trigger`/`skip` are special (collision/sensor only, not drawn) — planned.
- **Entity classnames** (add spawns in `MapLoad`, planned): `worldspawn` (level geometry), `info_player_start`
  (`origin`+`angle`), and later `monster_*`, `weapon_*`, `light`, `trigger_hurt`, `func_*`.

## Author a box brush by hand
For an axis-aligned box `[x1,x2]×[y1,y2]×[z1,z2]`, one face per plane; any 3 distinct points **on** that
plane work (winding-agnostic). Minimal example is `maps/box.map`. A room is 6 such box brushes
(floor, ceiling, 4 walls) with the interior hollow.

## Generate maps programmatically
Prefer emitting `.map` text from a small script (Python/JS in `tools/`) over hand-typing many brushes — a
box-brush emitter is ~20 lines. Then validate (below). This is how we scale level/greybox content without a
TrenchBroom CLI.

## Validate a .map
- **Fast**: it must parse and build — add/adjust a case in `tests/test_mapparser.cpp` /
  `tests/test_brushgeometry.cpp`, or load it in-game once rendering lands (M1b).
- **Visual**: once the world renders, `adv-build` screenshots it.
- Watch for: degenerate/duplicate planes, non-convex brushes (Valve brushes must be convex), Z-up mistakes,
  a texture with no matching PNG.

## TrenchBroom setup (for interactive authoring)
- Add a game config + `adventure.fgd` (entity definitions) so the editor offers our classnames (planned).
- Point TrenchBroom's texture collection at `assets/textures/` (loose PNGs) so previews match the game.
- Export as **Valve** format (the default 220), not Standard/Quake2.
