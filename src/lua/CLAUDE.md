# src/lua — CLAUDE.md

Sandboxed Lua scripting. VM = vendored minilua (Lua 5.5); binding = LuaBridge3.

| File | Responsibility |
|------|----------------|
| `ScriptEngine.{h,cpp}` | Public API behind a pimpl (`sScript`): `init/runString/runFile/selfTest/luaMemoryBytes`, `evalNumber`/`evalString` (evaluate `return (expr)` under the sandbox → number/string, else the default). Core (state, sandbox, watchdog, chunk loading) via the **raw Lua C API** — no LuaBridge here. |
| `ScriptEngineImpl.h` | Internal state struct (`lua_State`, watchdog counters, sandbox ref). Shared by the two `.cpp`; not public. |
| `Bindings.cpp` | **The only TU that includes LuaBridge.h** (big-obj, exceptions off). Registers the host API (`bindHostApi`). Add C++↔Lua gameplay bindings here. |

## Sandbox (don't weaken it)
- Chunks run under a whitelisted `_ENV`; `_G`, `io`, `os.execute`, `require`, `load`, `dofile`, `debug`,
  `package` are absent. `os` exposes only time helpers.
- Chunks load as **text only** (`"t"`) — bytecode rejected. Instruction-count watchdog (`lua_sethook`)
  kills runaway loops. A **capped allocator** (`lua_newstate` custom `lua_Alloc`, 64 MB) makes a memory
  bomb raise "not enough memory" instead of exhausting the process. Every untrusted call crosses `lua_pcall`.
- The sandbox is a **security boundary** — `tests/test_sandbox.cpp` adversarially tries to escape it
  (reach `_G`/`io`/loaders, infinite loop, memory bomb) and asserts containment. `selfTest()` re-checks on
  boot. If you touch `buildSandbox`/the allocator/watchdog, keep both test files green.

## Extending (the C++/Lua boundary)
- Keep LuaBridge confined to `Bindings.cpp`. Never expose `entt::registry` or raw pointers to Lua — pass a
  narrow, value-based handle backed by a stable `entt::entity`.
- Data + policy (weapon defs, enemy AI, encounters, tuning) live in `scripts/`; hot path stays in C++.
