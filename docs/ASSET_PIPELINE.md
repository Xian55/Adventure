# Asset Production Pipeline

How to make GRAVEN-style assets **at scale** without hand-fiddling each file. The retro look is a
*constraint*, and constraints automate well: fixed palette, small textures, point filtering, strict names.
The engine already renders at 480×270 and point-upscales — so source art can be small and cheap.

## Art targets (the constraints that make batching possible)
- **Palette:** one master palette (dark gothic, ~32–64 colours). Every texture/sprite is quantized to it.
  Store it as `assets/palette/gothic.png` (a 1×N or N×1 strip) and/or a GIMP/`.gpl` file.
- **Texture size:** wall/floor/prop textures **64–256 px**, power-of-two, tileable. Bigger is wasted — the
  frame is 480×270.
- **Filtering:** point/nearest everywhere (engine sets `TEXTURE_FILTER_POINT`). Author with hard pixels.
- **Sprites (enemies/pickups):** billboards. Enemies get **8 facing directions** × frames per animation,
  packed into one atlas per enemy. Transparent background (alpha), consistent ground anchor.
- **Format:** loose **PNG**. The texture's filename **is** its name in TrenchBroom and in the engine.

## Directory & naming (engine and editor must agree)
```
assets/
  palette/gothic.png
  textures/            # brush textures; filename = TrenchBroom texture name
    stone_wall_01.png  floor_tile_01.png  trim_gold_01.png
    clip.png  trigger.png            # special (collision/sensor only, not drawn)
  sprites/             # billboard atlases
    skeleton.png  skeleton.json      # atlas + frame/anim metadata
    pickup_sword.png
  viewmodel/
    sword.png  torch.png
```
Rules: lowercase `snake_case`, category prefix (`stone_`, `floor_`, `trim_`, `pickup_`), zero-padded
variants (`_01`, `_02`). A texture named `stone_wall_01` in a `.map` face loads `textures/stone_wall_01.png`.

## The scale workflow (generate → normalize → drop in → it just appears)
1. **Generate** the raw image (any of):
   - **Hand pixel art** in Aseprite (best for hero props; scriptable via its CLI/Lua).
   - **AI generation** (Stable Diffusion / ComfyUI) for bulk material/prop concepts — then *always* run the
     normalize pass so it conforms to the palette and grid.
   - **Photo/material sources** downscaled.
2. **Normalize** — one batch step enforces the look and the rules (see the tools below): downscale to the
   target size, **quantize to the master palette**, optionally add ordered dithering, strip metadata,
   validate the name/size, write to `assets/textures|sprites/`.
3. **Drop in** — the build's `copy_assets` target stages `assets/` next to the exe automatically. TrenchBroom
   points its texture collection at the same `assets/textures/`, so the editor preview matches the game.
4. **Validate** — a checker script confirms every texture referenced by a `.map` exists as a PNG, and every
   PNG is on-palette and correctly sized, before you commit.

Because steps 2–4 are scripts, adding 50 textures is: dump 50 raws in an inbox, run one command, commit.

## Tooling (lives in `tools/`)
Keep the generators/normalizers as small, rerunnable scripts (Python + Pillow, or ImageMagick). Suggested:
- `tools/normalize_texture.py <in> <out> --size 128` — resize (nearest), quantize to `gothic.png`, save.
- `tools/build_atlas.py <frames_dir> <out.png> <out.json>` — pack sprite frames + emit frame/anim JSON.
- `tools/validate_assets.py` — palette/size/name lint + `.map`→texture existence check (run in CI later).

**Palette-quantize (Pillow) sketch** — the core of `normalize_texture.py`:
```python
from PIL import Image
pal_img = Image.open("assets/palette/gothic.png").convert("RGB")
pal = pal_img.getdata()                 # list of (r,g,b)
palette = Image.new("P", (1, 1)); flat = []
for c in pal: flat += list(c)
flat += [0,0,0]*(256-len(pal))
palette.putpalette(flat)
src = Image.open(inp).convert("RGB").resize((size, size), Image.NEAREST)
out = src.quantize(palette=palette, dither=Image.Dither.NONE)  # or FLOYDSTEINBERG
out.convert("RGB").save(outp)
```

**ImageMagick one-liners** (quick bulk work):
```bash
# quantize a folder to the master palette + resize to 128, nearest-neighbour
for f in inbox/*.png; do
  magick "$f" -filter point -resize 128x128 -remap assets/palette/gothic.png \
    "assets/textures/$(basename "$f")"
done
```

**Aseprite CLI** (batch-export sprite sheets from `.aseprite` sources). Referenced via the **`ASEPRITE`**
env var (machine-independent) — see the `adv-sprite` skill:
```bash
"$ASEPRITE" -b sprites_src/skeleton.aseprite \
  --sheet assets/sprites/skeleton.png --data assets/sprites/skeleton.json \
  --sheet-type packed --list-tags
```

## TrenchBroom integration
- Add `assets/textures/` as a **loose-image texture collection** so the editor shows real game textures.
- Author `adventure.fgd` (entity definitions) so the editor offers our classnames with correct keys.
- Special textures (`clip`, `trigger`, `skip`) render nothing in-engine but still drive collision/sensors.

## Rules of thumb
- **Never hand-place what a script can stamp.** Generation and packing are code.
- **One palette to rule them all** — cohesion comes from the shared palette + point filtering + fog, not
  from per-asset polish.
- **Name is contract.** A rename is a code change (it's referenced by `.map` and the engine).
- **Validate before commit.** On-palette, right size, referenced textures exist.
