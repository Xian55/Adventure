# src/lua — CLAUDE.md

Sandboxed Lua scripting. VM = vendored minilua (Lua 5.5); binding = LuaBridge3.

| File | Responsibility |
|------|----------------|
| `ScriptEngine.{h,cpp}` | Public API behind a pimpl (`sScript`): `init/runString/runFile/selfTest/luaMemoryBytes`. Core (state, sandbox, watchdog, chunk loading) via the **raw Lua C API** — no LuaBridge here. |
| `ScriptEngineImpl.h` | Internal state struct (`lua_State`, watchdog counters, sandbox ref). Shared by the two `.cpp`; not public. |
| `Bindings.cpp` | **The only TU that includes LuaBridge.h** (big-obj, exceptions off). Registers the host API (`bindHostApi`). Add C++↔Lua gameplay bindings here. |

## Sandbox (don't weaken it)
- Chunks run under a whitelisted `_ENV`; `_G`, `io`, `os.execute`, `require`, `load`, `dofile`, `debug`,
  `package` are absent. `os` exposes only time helpers.
- Chunks load as **text only** (`"t"`) — bytecode rejected. Instruction-count watchdog (`lua_sethook`)
  kills runaway loops. Every untrusted call crosses `lua_pcall`.
- `selfTest()` asserts all of the above on boot; `tests/test_scriptengine.cpp` locks it in. If you touch
  `buildSandbox`, keep the tests green.

## Extending (the C++/Lua boundary)
- Keep LuaBridge confined to `Bindings.cpp`. Never expose `entt::registry` or raw pointers to Lua — pass a
  narrow, value-based handle backed by a stable `entt::entity`.
- Data + policy (weapon defs, enemy AI, encounters, tuning) live in `scripts/`; hot path stays in C++.
