---
name: adv-test
description: Build and run the Adventure headless unit-test suite (doctest via CTest). Use after changing anything in adventure_lib (Lua engine, metrics, systems) to check for regressions before committing.
---

# adv-test — build + run the test suite

The tests link `adventure_lib` and never open a window, so they run anywhere.

## Steps

```bash
# build (configure first if needed: cmake --preset mingw-debug)
cmake --build build/mingw-debug

# run via CTest
ctest --test-dir build/mingw-debug --output-on-failure
```

For richer output, run the test exe directly (doctest reporter):
```bash
./build/mingw-debug/adventure_tests.exe            # summary
./build/mingw-debug/adventure_tests.exe -s         # list successful assertions too
./build/mingw-debug/adventure_tests.exe -tc="watchdog*"   # filter by test-case name
```

## Notes
- MinGW binaries statically link the GCC/libstdc++/winpthread runtimes; if a run ever fails with
  `0xc0000139` (entrypoint not found), a wrong runtime DLL is on PATH — the static link should prevent it.
- Negative-path tests intentionally print Lua errors (watchdog kill, syntax error, `error('boom')`); a
  green summary (`Status: SUCCESS!`) is what matters.
- Add tests next to the code: put `.cpp` in `tests/`, list it in `adventure_tests` in `CMakeLists.txt`.
  Keep logic testable by injecting time/IO (see `Metrics::setClock`).
