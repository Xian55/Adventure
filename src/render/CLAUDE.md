# src/render — CLAUDE.md

Everything that draws. raylib lives here.

| File | Responsibility |
|------|----------------|
| `Renderer.{h,cpp}` | Low-res pipeline: `beginScene` → (caller draws in `BeginMode3D`) → `endScene` → `postProcess` (palette+dither shader) → `blit` (point-upscale to window). |
| `Shaders.h` | Inline GLSL (GL330). Post shader = palette quantize + Bayer 4×4 dither, indexed by low-res texel. |
| `MetricsOverlay.{h,cpp}` | Draws a `Metrics` snapshot at native res. Presentation only; reads, never mutates. |

## Pipeline notes (gotchas)
- Two low-res `RenderTexture2D` (scene, post), both `TEXTURE_FILTER_POINT`. Render at `kLowW×kLowH`.
- RenderTexture Y-flip: source rects use **negative height**. Net one flip scene→screen.
- `blit` targets `GetRenderWidth/Height` (real framebuffer), **not** the logical window — HiDPI would
  otherwise leave black borders.
- `main` owns `BeginDrawing`/`EndDrawing` around `blit` so it can draw native-res HUD/overlay on top.

## Coming (M1/M2)
`Billboard` (Y-axis camera-facing sprite quads, depth-sorted), `Viewmodel` (rlgl torch+sword overlay,
depth-test off), `Frustum` (Gribb-Hartmann cull). Enemies use a `RenderKind` seam: billboard now, 3D model
later — the swap should touch only the render system.
