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

## Performance gate (benchmarks)
The `adventure_bench` target is a headless micro-benchmark harness that returns nonzero if any case
exceeds its ms budget. It's registered as a CTest test **only for optimized builds** (Debug is too noisy),
so run it in Release:
```bash
cmake --preset mingw-release && cmake --build build/mingw-release
ctest --test-dir build/mingw-release -C Release --output-on-failure   # tests + bench gate
./build/mingw-release/adventure_bench.exe                             # raw numbers
```
Add a bench case for any new hot-path code: `ADV_BENCH(name, iters, budgetMs) { ...one iteration... }`
in a `bench/*.cpp`, listed in the `adventure_bench` target. Budgets are ceilings with headroom, not
targets — they catch regressions.

To profile the real render loop headlessly: `ADVENTURE_PROFILE=300 ./adventure.exe` writes `profile.csv`
(frame avg/p50/p95/max, RSS, Lua bytes, per-section ms).

## Sanitizers (ASan + UBSan)
CI runs the headless tests under AddressSanitizer + UndefinedBehaviorSanitizer (Linux/gcc) — the net that
catches use-after-free / UB our unit tests can't. To reproduce locally on Linux/WSL:
```bash
cmake -S . -B build/asan -G Ninja -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_C_FLAGS="-fsanitize=address,undefined -fno-sanitize-recover=all" \
  -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-sanitize-recover=all" \
  -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address,undefined"
cmake --build build/asan --target adventure_tests
ASAN_OPTIONS=detect_leaks=0 ctest --test-dir build/asan -R adventure_tests --output-on-failure
```
The Lua sandbox is a security boundary — `tests/test_sandbox.cpp` adversarially tries to escape/exhaust it;
keep it green when touching `src/lua`.

## Notes
- MinGW binaries statically link the GCC/libstdc++/winpthread runtimes; if a run ever fails with
  `0xc0000139` (entrypoint not found), a wrong runtime DLL is on PATH — the static link should prevent it.
- Negative-path tests intentionally print Lua errors (watchdog kill, syntax error, `error('boom')`); a
  green summary (`Status: SUCCESS!`) is what matters.
- Add tests next to the code: put `.cpp` in `tests/`, list it in `adventure_tests` in `CMakeLists.txt`.
  Keep logic testable by injecting time/IO (see `Metrics::setClock`).
