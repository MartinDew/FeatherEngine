cmake_minimum_required(VERSION 3.15)

include(tools/options.cmake)

project(feather VERSION 1.0.0 LANGUAGES CXX)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Set C++ standard
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set default build type to Release if not specified
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif ()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Configuration types for multi-config generators
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;Development" CACHE STRING "" FORCE)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(CMAKE_MAP_IMPORTED_CONFIG_Development RelWithDebInfo CACHE STRING "" FORCE)

set(PROJECT_HEADERS
        # Root Core files
        # main
        core/main/launch_settings.h
        core/main/window.h
        core/main/engine.h
        # Math
        core/math/transform.h
        core/math/math_defs.h
        #Framework
        core/framework/assert.hpp
        core/framework/static_string.hpp
        core/framework/delegate.h
        core/framework/containers.h
        core/framework/static_indexed_array.h
        core/framework/functions.h

        #Rendering
        core/rendering/rendering_server.h
        core/rendering/renderer.h
        core/main/notification.h
)

# Source files (shared between both targets)
set(PROJECT_SOURCES
        # Root
        core/main/main.cpp
        core/main/launch_settings.cpp
        core/main/window.cpp
        core/main/engine.cpp
        # Math
        core/math/transform.cpp
        core/math/math_defs.cpp
        # Rendering
        core/rendering/renderer.cpp
        core/rendering/rendering_server.cpp
)

# Editor target
add_executable(Editor ${PROJECT_SOURCES} ${PROJECT_HEADERS} )

set_target_properties(Editor PROPERTIES OUTPUT_NAME ${PROJECT_NAME}.editor)
set_target_properties(Editor PROPERTIES ENABLE_EXPORTS ON)
target_compile_definitions(Editor PUBLIC EDITOR_BUILD=1
        $<$<CONFIG:Debug>:BETA>
        $<$<CONFIG:Development>:BETA>
)

target_include_directories(Editor PUBLIC ${CMAKE_SOURCE_DIR} core)

# Compiler options for Editor
target_compile_options(Editor PRIVATE
        $<$<CONFIG:Debug>:-g -O0>
        $<$<CONFIG:Release>:-O3 -DNDEBUG>
        $<$<CONFIG:Development>:-g -O2>
)

# Standalone target
add_executable(Standalone ${PROJECT_SOURCES} ${PROJECT_HEADERS} )
target_include_directories(Standalone PUBLIC core ${CMAKE_SOURCE_DIR})

set_target_properties(Standalone PROPERTIES OUTPUT_NAME ${PROJECT_NAME}.standalone)
set_target_properties(Standalone PROPERTIES ENABLE_EXPORTS ON)

target_compile_definitions(Standalone PRIVATE EDITOR_BUILD=0
        $<$<CONFIG:Debug>:BETA>
        $<$<CONFIG:Development>:BETA>
)

# Compiler options for Standalone
target_compile_options(Standalone PRIVATE
        $<$<CONFIG:Debug>:-g -O0>
        $<$<CONFIG:Release>:-O3 -DNDEBUG>
        $<$<CONFIG:Development>:-g -O2>
)


add_subdirectory(thirdparty)

# Platform-specific settings
if (MSVC)
    target_compile_options(Editor PRIVATE /W4 /wd4100)
    target_compile_options(Standalone PRIVATE /W4 /wd4100)

    # MSVC-specific config flags
    target_compile_options(Editor PRIVATE
            $<$<CONFIG:Debug>:/Od /Zi>
            $<$<CONFIG:Release>:/O2 /DNDEBUG>
            $<$<CONFIG:Development>:/O2 /Zi>
    )
    target_compile_options(Standalone PRIVATE
            $<$<CONFIG:Debug>:/Od /Zi>
            $<$<CONFIG:Release>:/O2 /DNDEBUG>
            $<$<CONFIG:Development>:/O2 /Zi>
    )

    # NOMINMAX and LEAN_AND_MEAN to reduce Windows headers bloat
    target_compile_definitions(Editor PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
    target_compile_definitions(Standalone PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)

    # embed internal_resources folder
    target_compile_options(Editor PUBLIC "\/clang:--embed-dir=${CMAKE_SOURCE_DIR}/internal_resources")
    target_compile_options(Standalone PUBLIC "\/clang:--embed-dir=${CMAKE_SOURCE_DIR}/internal_resources")

else ()
    target_compile_options(Editor PRIVATE -Wall -Wextra -pedantic -Wno-unused-parameter)
    target_compile_options(Standalone PRIVATE -Wall -Wextra -pedantic -Wno-unused-parameter)
    # embed internal_resources folder
    target_compile_options(Editor PUBLIC "--embed-dir=${CMAKE_SOURCE_DIR}/internal_resources")
    target_compile_options(Standalone PUBLIC "--embed-dir=${CMAKE_SOURCE_DIR}/internal_resources")
endif ()

add_subdirectory(modules)

if (${enable_vex_renderer}})
        vex_setup_runtime(Editor)
        vex_setup_runtime(Standalone)
endif()

# Print build configuration
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")