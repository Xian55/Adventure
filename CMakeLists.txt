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

# Warnings-as-errors on OUR targets only (deps are SYSTEM / separate libs, exempt).
if(MSVC)
    set(ADV_WARN /W4 /WX)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS) # allow portable sscanf/getenv (not the _s variants)
else()
    set(ADV_WARN -Wall -Wextra -Werror)
endif()

# --- minilua: the vendored Lua 5.5 amalgamation as its own lib (never warning-gated) ---
add_library(minilua STATIC deps/minilua/minilua_impl.c)
target_include_directories(minilua SYSTEM PUBLIC deps/minilua)

# --- adventure_lib: all game logic as a static library so it can be unit-tested without a window ---
set(ADVENTURE_LIB_SOURCES
    src/core/Platform.cpp
    src/core/Metrics.cpp
    src/core/ProfileReport.cpp
    src/world/MapParser.cpp
    src/world/BrushGeometry.cpp
    src/world/CollisionWorld.cpp
    src/player/PlayerController.cpp
    src/player/JumpMeter.cpp
    src/combat/Melee.cpp
    src/combat/CombatSystem.cpp
    src/combat/Rage.cpp
    src/combat/Destructible.cpp
    src/items/Item.cpp
    src/items/Inventory.cpp
    src/items/Pickup.cpp
    src/items/Container.cpp
    src/items/Collide.cpp
    src/render/Renderer.cpp
    src/render/MetricsOverlay.cpp
    src/render/WorldRenderer.cpp
    src/render/Viewmodel.cpp
    src/render/Billboard.cpp
    src/render/Prop.cpp
    src/lua/ScriptEngine.cpp
    src/lua/Bindings.cpp                 # the only TU that includes LuaBridge
)

add_library(adventure_lib STATIC ${ADVENTURE_LIB_SOURCES})
target_include_directories(adventure_lib PUBLIC src)
target_include_directories(adventure_lib SYSTEM PUBLIC deps/luabridge) # vendored header, exempt from -Werror
target_link_libraries(adventure_lib PUBLIC raylib EnTT::EnTT minilua)
target_compile_options(adventure_lib PRIVATE ${ADV_WARN})
if(WIN32)
    target_link_libraries(adventure_lib PUBLIC psapi)   # GetProcessMemoryInfo / GetProcessTimes
endif()

# The LuaBridge + minilua-decls TU is large; give it a big object-file section.
if(MSVC)
    set_source_files_properties(src/lua/Bindings.cpp PROPERTIES COMPILE_OPTIONS "/bigobj")
elseif(MINGW)
    # -Wa,-mbig-obj is a COFF/PE (Windows) assembler flag; ELF (Linux/macOS) has no 64k-section limit.
    set_source_files_properties(src/lua/Bindings.cpp PROPERTIES COMPILE_OPTIONS "-Wa,-mbig-obj")
endif()

# --- adventure: thin executable ---
add_executable(adventure src/main.cpp)
target_link_libraries(adventure PRIVATE adventure_lib)
target_compile_options(adventure PRIVATE ${ADV_WARN})

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
    tests/test_geometry_golden.cpp
    tests/test_collisionworld.cpp
    tests/test_playercontroller.cpp
    tests/test_jumpmeter.cpp
    tests/test_melee.cpp
    tests/test_combat.cpp
    tests/test_rage.cpp
    tests/test_billboard.cpp
    tests/test_hazard.cpp
    tests/test_destructible.cpp
    tests/test_inventory.cpp
    tests/test_pickup.cpp
    tests/test_container.cpp
    tests/test_collide.cpp
    tests/test_sandbox.cpp
)
target_link_libraries(adventure_tests PRIVATE adventure_lib doctest::doctest)
target_compile_options(adventure_tests PRIVATE ${ADV_WARN})
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
target_compile_options(adventure_bench PRIVATE ${ADV_WARN})
# The perf gate is meaningful in Release; register it as a CTest test only for optimized builds so
# Debug runs (slow, noisy) don't produce false regressions.
add_test(NAME adventure_bench
    CONFIGURATIONS Release RelWithDebInfo
    COMMAND adventure_bench)
