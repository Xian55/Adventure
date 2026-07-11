# src/render — CLAUDE.md

Everything that draws. raylib lives here.

| File | Responsibility |
|------|----------------|
| `Renderer.{h,cpp}` | Low-res pipeline: `beginScene` → (caller draws in `BeginMode3D`) → `endScene` → `postProcess` (palette+dither shader) → `blit` (point-upscale to window). |
| `Shaders.h` | Inline GLSL (GL330). Post shader = palette quantize + Bayer 4×4 dither. World shader = textured + vertex-light + exp2 distance fog + alpha-test discard. |
| `WorldRenderer.{h,cpp}` | Uploads `world::WorldGeometry` (per-texture `MeshData`) to raylib meshes + the world shader; `draw(camPos)` sets fog/camera uniforms. Missing texture PNG → a generated checker placeholder (assets are local/optional). |
| `Viewmodel.{h,cpp}` | First-person torch (left) + sword (right) + a **boot kick** (thrusts up from bottom-centre on F, `kick` param 1→0), drawn over the world with depth-test off (own near camera, `rlgl`) inside the low-res RT. Procedural placeholder geometry + torch flicker + movement bob, until sprites/models exist. |
| `Billboard.{h,cpp}` | Enemy sprites — Y-facing upright billboards (`DrawBillboardPro`, anchored at the **feet**), **depth-sorted far→near** (`depthSortEnemies`, pure/tested), state-tinted, with a contact-shadow disc. Skeleton sprite from `assets/sprites/skeleton.png` or a generated placeholder. The `RenderKind` seam: draws `Billboard`-kind enemies; `Box`-kind stay debug cubes (toggle **B**). |
| `Mech.{h,cpp}` | Doors / levers / pressure plates — placeholder 3D (sliding box door, throw-handle lever, sinking pad). Reads `mech::Door`/`Lever`/`Plate`; presentation only. |
| `Prop.{h,cpp}` | Destructible props + loot pickups + chests — placeholder 3D: barrels/kegs = cylinders (hoop bands), crates = cubes, chests = box + hinged lid (+ lock plate); a debris puff on break; a bobbing item orb (coloured by item kind). Reads `combat::Destructible`/`items::Pickup`/`items::Container`; presentation only. |
| `MetricsOverlay.{h,cpp}` | Draws a `Metrics` snapshot at native res. Presentation only; reads, never mutates. |

## Pipeline notes (gotchas)
- Two low-res `RenderTexture2D` (scene, post), both `TEXTURE_FILTER_POINT`. Render at `kLowW×kLowH`.
- RenderTexture Y-flip: source rects use **negative height**. Net one flip scene→screen.
- `blit` targets `GetRenderWidth/Height` (real framebuffer), **not** the logical window — HiDPI would
  otherwise leave black borders.
- `main` owns `BeginDrawing`/`EndDrawing` around `blit` so it can draw native-res HUD/overlay on top.

## Billboard gotchas
- `DrawBillboardPro` anchors the quad at `position` as its **bottom-left corner** (points `0→right→up+
  right→up`), *not* the center — pass the enemy's **feet** + `origin.x = size.x/2` to stand it centered.
- Billboards cast no shadow → they look like they float. Two fixes used: a **contact-shadow disc** on the
  floor, and enemies are **ground-snapped at spawn** (they have no gravity; `main` probes down with
  `CollisionWorld::overlaps`). Runtime enemy gravity/physics is still future.
- Transparency: sprites are alpha-blended + drawn far→near so overlaps read right (no alpha-test shader yet).

## Coming (M2+)
`Frustum` (Gribb-Hartmann cull). Real skeleton art via `adv-sprite` + animation frames (currently one frame,
state via tint). Torch flame will drive a `LightSource`. WorldRenderer per-mesh frustum culling as maps grow.
`RenderKind` gains a 3D-model path later — the swap touches only the render side.
