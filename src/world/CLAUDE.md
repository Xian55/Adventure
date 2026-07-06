# src/world — CLAUDE.md

Level loading: TrenchBroom/Quake `.map` (Valve 220) → engine geometry + collision. Pure/headless (no
raylib window), fully unit-tested. Format authority: the `adv-map` skill.

| File | Responsibility |
|------|----------------|
| `MapTypes.h` | Data types: `Face`/`Brush`/`Entity`/`MapData` (map space) and `MeshData`/`CollisionBrush`/`WorldGeometry` (engine space). |
| `MapParser.{h,cpp}` | Valve 220 text → `MapData`. Tolerant tokenizer+parser: `parseMap` returns `ok=false`+error on malformed input, **never throws**. Keeps raw map coords. |
| `BrushGeometry.{h,cpp}` | `MapData` → `WorldGeometry`: per-brush convex faces via plane clipping → per-texture `MeshData` + collision planes. Applies Z-up→Y-up + `kMapScale`. |

## Key facts / gotchas
- **Winding-agnostic**: face normals are oriented outward from the brush interior (average of face points),
  so authored point order doesn't matter.
- **Convex hull by clipping**: each face = a big seed quad on its plane, clipped by every other face's
  half-space (Sutherland-Hodgman). `< 3` points ⇒ face dropped.
- **Coords**: `engine = (x, z, -y) * kMapScale` (`kMapScale = 1/32`), one place in `BrushGeometry::toEngine`.
- **Collision** is stored as outward face planes (`Vector4` xyz=normal, w=d) + AABB — the movement system
  (M1c) tests an AABB against these via Minkowski-expanded planes. Render mesh is separate (per texture).
- Parser holds a **reference** to its token vector — keep the tokens alive for the Parser's lifetime (a
  dangling temporary here caused an intermittent bug; see `parseMap`).
- Vertex color currently bakes a fake directional shade so faces read in 3D before real lighting.

## Coming
`MapLoad` (classname → ECS spawn table), `render/WorldRenderer` (upload `MeshData` → raylib meshes + world
fog/vertex-light shader), `CollisionWorld` (AABB-vs-brush queries for movement).
