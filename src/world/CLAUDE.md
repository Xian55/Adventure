# src/world — CLAUDE.md

Level loading: TrenchBroom/Quake `.map` (Valve 220) → engine geometry + collision. Pure/headless (no
raylib window), fully unit-tested. Format authority: the `adv-map` skill.

| File | Responsibility |
|------|----------------|
| `MapTypes.h` | Data types: `Face`/`Brush`/`Entity`/`MapData` (map space) and `MeshData`/`CollisionBrush`/`WorldGeometry` (engine space). |
| `MapParser.{h,cpp}` | Quake `.map` text → `MapData`. Handles **both Valve 220 and Standard Quake** face formats (Standard synthesizes tex axes from the normal via the baseaxis table) — so real id/Arcane-Dimensions maps load. Tolerant: `parseMap` returns `ok=false`+error on malformed input, **never throws**. |
| `BrushGeometry.{h,cpp}` | `MapData` → `WorldGeometry`: per-brush convex faces via plane clipping → per-texture `MeshData` + collision planes. Applies Z-up→Y-up + `kMapScale`. `mapToEngine()` for entity placement. **`trigger_*` entities are skipped** (sensors, not solid). `buildHazards()` extracts `trigger_hurt` brushes as engine-space `Hazard` AABBs; `hazardDamageAt(p)` = summed dmg/sec at a point. |
| `CollisionWorld.{h,cpp}` | Holds the collision brushes; `overlaps(center, halfExtents)` = AABB vs any convex brush (broadphase AABB filter + Minkowski-expanded plane test). Used by `player/PlayerController`. |

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

## Triggers / sensors
- `trigger_*` classnames are **not solid and not rendered** — `buildWorld` skips them so they don't become
  walls or checker-textured boxes. `trigger_hurt` → `Hazard{min,max,damagePerSec}` (engine AABB) via
  `buildHazards`; `dmg` key = damage/sec. `main` renders each as a red lava surface + wire outline, and
  `combat::applyHazards` / `hazardDamageAt` damage enemies + the player whose feet are inside.
- AABB is derived from the brush's clipped convex hull, so rotated/non-box triggers still bound correctly.

## Coming
`MapLoad` (classname → ECS spawn table), more trigger verbs (`trigger_multiple`/`func_*` → `sScript.fire`),
broadphase acceleration (BVH/grid) when maps grow — see the partitioning plan in `docs/design/ARCHITECTURE.md`.
