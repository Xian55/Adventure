# src/render — CLAUDE.md

Everything that draws. raylib lives here.

| File | Responsibility |
|------|----------------|
| `Renderer.{h,cpp}` | Low-res pipeline: `beginScene` → (caller draws in `BeginMode3D`) → `endScene` → `postProcess` (palette+dither shader) → `blit` (point-upscale to window). |
| `Shaders.h` | Inline GLSL (GL330). Post shader = palette quantize + Bayer 4×4 dither. World shader = textured + vertex-light + exp2 distance fog + alpha-test discard. |
| `WorldRenderer.{h,cpp}` | Uploads `world::WorldGeometry` (per-texture `MeshData`) to raylib meshes + the world shader; `draw(camPos)` sets fog/camera uniforms. Missing texture PNG → a generated checker placeholder (assets are local/optional). |
| `Viewmodel.{h,cpp}` | First-person torch (left) + sword (right), drawn over the world with depth-test off (own near camera, `rlgl`) inside the low-res RT. Procedural placeholder geometry + torch flicker + movement bob, until sprites/models exist. |
| `MetricsOverlay.{h,cpp}` | Draws a `Metrics` snapshot at native res. Presentation only; reads, never mutates. |

## Pipeline notes (gotchas)
- Two low-res `RenderTexture2D` (scene, post), both `TEXTURE_FILTER_POINT`. Render at `kLowW×kLowH`.
- RenderTexture Y-flip: source rects use **negative height**. Net one flip scene→screen.
- `blit` targets `GetRenderWidth/Height` (real framebuffer), **not** the logical window — HiDPI would
  otherwise leave black borders.
- `main` owns `BeginDrawing`/`EndDrawing` around `blit` so it can draw native-res HUD/overlay on top.

## Coming (M2+)
`Billboard` (Y-axis camera-facing sprite quads, depth-sorted), `Frustum` (Gribb-Hartmann cull). Viewmodel
swing animation driven by `WeaponState` (M2); torch flame will drive a `LightSource`. WorldRenderer gains
per-mesh frustum culling as maps grow. Enemies use a `RenderKind` seam: billboard now, 3D model later — the
swap touches only the render side.
