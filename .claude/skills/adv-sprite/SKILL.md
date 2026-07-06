---
name: adv-sprite
description: Produce/export game sprites (billboard atlases, viewmodels) with Aseprite via the ASEPRITE env var, so scripts stay machine-independent. Use when creating or batch-exporting sprite sheets, or setting up Aseprite CLI automation for the asset pipeline.
---

# adv-sprite — Aseprite sprite export

Aseprite is referenced via the **`ASEPRITE`** environment variable (User scope), not a hardcoded path, so
this works on any machine that sets it. Current value points at `Aseprite.exe`.

## Prerequisite
`ASEPRITE` must be set. Check / set (PowerShell):
```powershell
[Environment]::GetEnvironmentVariable("ASEPRITE","User")
# if empty:
[Environment]::SetEnvironmentVariable("ASEPRITE","C:\Program Files\Aseprite\Aseprite.exe","User")
```
A new shell picks it up. In bash it's `"$ASEPRITE"`; in PowerShell `$env:ASEPRITE`.

## Batch-export a sprite sheet + metadata
```bash
"$ASEPRITE" -b sprites_src/skeleton.aseprite \
  --sheet assets/sprites/skeleton.png \
  --data  assets/sprites/skeleton.json \
  --sheet-type packed --list-tags
```
- `-b` = batch (headless, no GUI). `--sheet` packed PNG, `--data` frame/anim JSON (tags = animations).
- Export every `.aseprite` in a folder:
  ```bash
  for f in sprites_src/*.aseprite; do
    "$ASEPRITE" -b "$f" --sheet "assets/sprites/$(basename "${f%.aseprite}").png" \
      --data "assets/sprites/$(basename "${f%.aseprite}").json" --sheet-type packed --list-tags
  done
  ```

## Conventions (see docs/ASSET_PIPELINE.md)
- Sprites are billboards: enemies get 8 facing directions × frames, packed into one atlas per enemy,
  transparent background, consistent ground anchor.
- Point filtering + the fixed gothic palette. Run the palette-normalize step on any non-palette source.
- Output goes to `assets/` (gitignored — assets stay local for now); the build stages it next to the exe.
- Aseprite also scripts in Lua (`--script foo.lua`) for custom batch pipelines if needed.
