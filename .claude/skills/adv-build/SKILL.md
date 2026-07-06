---
name: adv-build
description: Build the Adventure game with a CMake preset, run it, and capture a clean in-game screenshot to visually verify a rendering or gameplay change. Use when asked to build and run, screenshot the game, or confirm a change looks right on screen. Handles the display-DPI screenshot crop so the image has no black borders.
---

# adv-build — build, run, and screenshot Adventure

Use this to confirm a change actually renders correctly (not just compiles). Works headlessly by driving
the game's built-in one-shot screenshot hook.

## Steps

1. **Configure (first time or after CMake changes):**
   ```bash
   cmake --preset mingw-debug     # or msvc-debug
   ```

2. **Build:**
   ```bash
   cmake --build build/mingw-debug
   ```
   (MSVC: `cmake --build build/msvc-debug --config Debug`.)

3. **Run with the screenshot hook.** The game reads `ADVENTURE_SHOT=<file>`, renders ~30 frames, writes the
   PNG, and exits (no manual window closing):
   ```bash
   cd build/mingw-debug
   ADVENTURE_SHOT=shot.png ./adventure.exe > run.log 2>&1
   cat run.log      # confirm boot: "ScriptEngine selfTest OK", no errors
   ```
   MSVC exe is at `build/msvc-debug/Debug/adventure.exe`.

4. **Crop the DPI over-capture, then view.** raylib's `TakeScreenshot` multiplies by the display DPI scale,
   so the PNG is larger than the framebuffer with black at top+right. The real content is the bottom-left
   `framebuffer` region. On a **125% display** (`dpi = 1.25`) crop to the bottom-left `1/dpi` of the image:
   ```bash
   python -c "from PIL import Image; d=1.25; im=Image.open('shot.png'); W,H=im.size; im.crop((0,int(H*(1-1/d)),int(W/d),H)).save('shot_crop.png'); print('cropped', Image.open('shot_crop.png').size)"
   ```
   Then Read `shot_crop.png` to view it.
   - **Adjust `d` to the machine's display scale** (100%→1.0, 125%→1.25, 150%→1.5). If unsure, the game logs
     it: temporarily print `GetWindowScaleDPI()`, or check Windows Display settings.

## Notes
- The cropped image shows the pixelated, dithered, fogged frame plus the F3 metrics overlay.
- If the build fetches deps (raylib/EnTT/doctest) on first configure, it needs network + a few minutes.
- To iterate on **feel** (movement/combat constants) you usually don't rebuild — edit `scripts/tuning.lua`.
