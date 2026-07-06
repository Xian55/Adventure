# Adventure — GRAVEN-style first-person dark-fantasy melee game (raylib 5.5 + C++17).
cmake_minimum_required(VERSION 3.24)
project(Adventure CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# MinGW: statically link the GCC/libstdc++/winpthread runtimes so binaries don't depend on
# whichever toolchain DLLs happen to be on PATH (fixes ctest 0xc0000139 entrypoint mismatches
# and makes the game distributable standalone).
if(MINGW)
    add_link_options(-static -static-libgcc -static-libstdc++)
endif()

include(cmake/deps.cmake)

# --- adventure_lib: all game logic as a static library so it can be unit-tested without a window ---
set(ADVENTURE_LIB_SOURCES
    src/core/Platform.cpp
    src/core/Metrics.cpp
    src/core/ProfileReport.cpp
    src/world/MapParser.cpp
    src/world/BrushGeometry.cpp
    src/render/Renderer.cpp
    src/render/MetricsOverlay.cpp
    src/render/WorldRenderer.cpp
    src/lua/ScriptEngine.cpp
    src/lua/Bindings.cpp                 # the only TU that includes LuaBridge
    deps/minilua/minilua_impl.c          # Lua 5.5 amalgamation, one C TU
)

add_library(adventure_lib STATIC ${ADVENTURE_LIB_SOURCES})
target_include_directories(adventure_lib PUBLIC
    src
    deps/minilua
    deps/luabridge
)
target_link_libraries(adventure_lib PUBLIC raylib EnTT::EnTT)
if(WIN32)
    target_link_libraries(adventure_lib PUBLIC psapi)   # GetProcessMemoryInfo / GetProcessTimes
endif()

# The LuaBridge + minilua-decls TU is large; give it a big object-file section.
if(MSVC)
    set_source_files_properties(src/lua/Bindings.cpp PROPERTIES COMPILE_OPTIONS "/bigobj")
else()
    set_source_files_properties(src/lua/Bindings.cpp PROPERTIES COMPILE_OPTIONS "-Wa,-mbig-obj")
endif()

# --- adventure: thin executable ---
add_executable(adventure src/main.cpp)
target_link_libraries(adventure PRIVATE adventure_lib)

# Stage runtime data next to the executable so relative paths resolve after
# ChangeDirectory(GetApplicationDirectory()).
add_custom_target(copy_assets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/scripts $<TARGET_FILE_DIR:adventure>/scripts
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets  $<TARGET_FILE_DIR:adventure>/assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/maps    $<TARGET_FILE_DIR:adventure>/maps
    VERBATIM)
add_dependencies(adventure copy_assets)

# --- adventure_tests: doctest unit tests over adventure_lib (headless: no window created) ---
enable_testing()
add_executable(adventure_tests
    tests/test_main.cpp
    tests/test_scriptengine.cpp
    tests/test_metrics.cpp
    tests/test_profilereport.cpp
    tests/test_mapparser.cpp
    tests/test_brushgeometry.cpp
)
target_link_libraries(adventure_tests PRIVATE adventure_lib doctest::doctest)
add_test(NAME adventure_tests COMMAND adventure_tests)

# --- adventure_bench: headless micro-benchmarks that fail (nonzero exit) on budget breach ---
add_executable(adventure_bench
    bench/bench_main.cpp
    bench/bench_lua.cpp
    bench/bench_metrics.cpp
    bench/bench_world.cpp
)
target_include_directories(adventure_bench PRIVATE bench)
target_link_libraries(adventure_bench PRIVATE adventure_lib)
# The perf gate is meaningful in Release; register it as a CTest test only for optimized builds so
# Debug runs (slow, noisy) don't produce false regressions.
add_test(NAME adventure_bench
    CONFIGURATIONS Release RelWithDebInfo
    COMMAND adventure_bench)
