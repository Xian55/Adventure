---
name: adv-new-module
description: Scaffold a new single-responsibility C++ module (header + source) in the Adventure codebase, wired into adventure_lib and (if it has testable logic) a matching test. Use when adding a new system, component group, or subsystem so it follows the project's SRP, style, and build conventions.
---

# adv-new-module — scaffold an SRP module

Creates a `.h/.cpp` pair under `src/<area>/`, following the house style, and wires it into the build.

## Before writing
Decide the module's **one** responsibility and its area:
- `core/` cross-cutting (config, timing, platform), `render/` drawing, `world/` levels/collision,
  `ecs/` + `ecs/systems/` entities/systems, `combat/` combat rules, `lua/` scripting.
- If you need "and" to describe it, split it. Keep `entt::` out of render/world/lua public headers.

## Steps

1. **Header** `src/<area>/<Name>.h` — namespaced, `#pragma once`, tabs + Allman braces, `m_` members,
   a top comment stating the single responsibility:
   ```cpp
   #pragma once

   namespace adventure
   {
       // <One sentence: what this is responsible for — and what it deliberately is NOT.>
       class <Name>
       {
           public:
               <Name>();
               // ...
           private:
               // m_...
       };
   }
   ```

2. **Source** `src/<area>/<Name>.cpp`:
   ```cpp
   #include "<area>/<Name>.h"

   namespace adventure
   {
       <Name>::<Name>() {}
   }
   ```

3. **Wire into the build** — add the `.cpp` to `ADVENTURE_LIB_SOURCES` in `CMakeLists.txt` (keep it in
   `adventure_lib`, not the exe, so it's testable). If the TU includes `<windows.h>`, keep raylib out of it
   (see `core/Platform.cpp`). If it's the only TU touching a heavy header, isolate it like `Bindings.cpp`.

4. **Test (if logic is headless-testable)** — add `tests/test_<name>.cpp`, list it in the `adventure_tests`
   sources in `CMakeLists.txt`, and make time/IO injectable so assertions are deterministic (see
   `Metrics::setClock`). Rendering/OS glue is verified by running the game (`adv-build`), not unit tests.

5. **Verify:** `cmake --preset mingw-debug && cmake --build build/mingw-debug` and, if you added tests,
   run `adv-test`.

## Conventions reference
See `docs/design/CONVENTIONS.md` (SRP, style, testing) and `docs/design/ARCHITECTURE.md` (the module seams).
