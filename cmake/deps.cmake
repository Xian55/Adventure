# Third-party dependencies, fetched/built once. Isolated from the root list so the
# top-level CMakeLists stays readable. Mirrors craft_raylib's raylib FetchContent setup.
include(FetchContent)

# Don't re-hit the network on every reconfigure once the tags are checked out.
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

# Some pinned deps (doctest v2.4.11) declare a cmake_minimum_required below what CMake 4.x
# accepts. Provide a floor so their sub-configure doesn't hard-error.
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

# --- raylib 5.5 (built from source, pinned tag) ---
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)
FetchContent_Declare(raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        5.5
    GIT_SHALLOW    ON)
FetchContent_MakeAvailable(raylib)

# --- EnTT (header-only ECS) ---
FetchContent_Declare(EnTT
    GIT_REPOSITORY https://github.com/skypjack/entt.git
    GIT_TAG        v3.13.2
    GIT_SHALLOW    ON)
FetchContent_MakeAvailable(EnTT)

# --- doctest (single-header test framework) ---
FetchContent_Declare(doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG        v2.4.11
    GIT_SHALLOW    ON)
FetchContent_MakeAvailable(doctest)

# Treat dependency headers as SYSTEM so their warnings don't trip our -Werror on our own code.
foreach(dep raylib EnTT doctest)
    if(TARGET ${dep})
        set_target_properties(${dep} PROPERTIES SYSTEM ON)
    endif()
endforeach()
