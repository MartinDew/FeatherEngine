cmake_minimum_required(VERSION 3.15)

# Configuration types for multi-config generators
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;Development" CACHE STRING "" FORCE)
# Set default build type to Release if not specified
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif ()
include(tools/options.cmake)

project(feather VERSION 1.0.0 LANGUAGES CXX)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Required for tools
find_package(Python3 COMPONENTS Interpreter REQUIRED)

if (${APPLE})
    add_compile_options("-fexperimental-library")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(CMAKE_MAP_IMPORTED_CONFIG_Development RelWithDebInfo CACHE STRING "" FORCE)

set(FEATHER_CORE_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/core/framework/callable.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/core/framework/reflected.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/core/framework/variant.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/core/framework/variant_array.cpp"

        "${CMAKE_CURRENT_SOURCE_DIR}/core/main/class_db.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/core/main/engine.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/core/main/engine_settings.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/core/main/feather_main.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/core/main/launch_settings.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/core/main/project_settings.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/core/main/window.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/core/main/simulation.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/core/main/world_sim.cpp"

        "${CMAKE_CURRENT_SOURCE_DIR}/core/math/math_defs.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/core/math/projection.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/core/math/transform.cpp"

        "${CMAKE_CURRENT_SOURCE_DIR}/core/rendering/mesh_data.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/core/rendering/renderer.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/core/rendering/rendering_server.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/core/rendering/render_scene.cpp"

        "${CMAKE_CURRENT_SOURCE_DIR}/core/resources/material.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/core/resources/material_format_loader.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/core/resources/mesh.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/core/resources/mesh_format_loader.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/core/resources/resource.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/core/resources/resource_format_loader.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/core/resources/resource_loader.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/core/resources/rid.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/core/resources/shader.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/core/resources/texture.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/core/resources/texture_format_loader.cpp"
)

set(GENERATED_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/core/framework/register_framework_types.gen.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/core/framework/register_framework_types.gen.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/core/main/register_main_types.gen.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/core/main/register_main_types.gen.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/core/math/register_math_types.gen.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/core/math/register_math_types.gen.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/core/rendering/register_rendering_types.gen.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/core/rendering/register_rendering_types.gen.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/core/resources/register_resources_types.gen.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/core/resources/register_resources_types.gen.h"
)

# Editor target
add_executable(Editor
        ${FEATHER_CORE_SOURCES}
        ${GENERATED_FILES})

set_target_properties(Editor PROPERTIES OUTPUT_NAME ${PROJECT_NAME}.editor)
# set_target_properties(Editor PROPERTIES ENABLE_EXPORTS ON)
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
add_executable(Standalone ${FEATHER_CORE_SOURCES} ${GENERATED_FILES})
target_include_directories(Standalone PUBLIC core ${CMAKE_SOURCE_DIR})

set_target_properties(Standalone PROPERTIES OUTPUT_NAME ${PROJECT_NAME}.standalone)
# set_target_properties(Standalone PROPERTIES ENABLE_EXPORTS ON)

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

# Generate embedded resource headers using #embed

add_custom_target(GenerateEmbeddedResources
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/generate_embedded_resources.py
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Generating embedded resource headers with #embed directives"
        VERBATIM
)

# Make both targets depend on resource generation
add_dependencies(Editor GenerateEmbeddedResources)
add_dependencies(Standalone GenerateEmbeddedResources)

# Bind generation of type registration functions to a custom target
add_custom_target(GenerateRegisterTypes
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/generate_core_registers.py --core-path ./core
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Generating type registration functions"
        BYPRODUCTS ${GENERATED_FILES}
        VERBATIM
)

add_dependencies(Editor GenerateRegisterTypes)
add_dependencies(Standalone GenerateRegisterTypes)

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

    # embed raw_resources folder
    #    target_compile_options(Editor PUBLIC "\/clang:--embed-dir=${CMAKE_SOURCE_DIR}/raw_resources")
    #    target_compile_options(Standalone PUBLIC "\/clang:--embed-dir=${CMAKE_SOURCE_DIR}/raw_resources")

else ()
    target_compile_options(Editor PRIVATE -Wall -Wextra -pedantic -Wno-unused-parameter)
    target_compile_options(Standalone PRIVATE -Wall -Wextra -pedantic -Wno-unused-parameter)
    # embed raw_resources folder

    #    target_compile_options(Editor PUBLIC "--embed-dir=${CMAKE_SOURCE_DIR}/raw_resources")
    #    target_compile_options(Standalone PUBLIC "--embed-dir=${CMAKE_SOURCE_DIR}/raw_resources")

    if (STATIC_CPP AND LINUX)
        # For a specific executable target

        target_link_libraries(Editor PRIVATE "-static-libgcc" "-static-libstdc++")
        target_link_libraries(Standalone PRIVATE "-static-libgcc" "-static-libstdc++")
        if (USE_LLVM)
            set_target_properties(Editor PROPERTIES LINK_SEARCH_START_STATIC ON)
            set_target_properties(Standalone PROPERTIES LINK_SEARCH_START_STATIC ON)

            set_target_properties(Editor PROPERTIES LINK_SEARCH_END_STATIC ON)
            set_target_properties(Standalone PROPERTIES LINK_SEARCH_END_STATIC ON)
        endif ()
    endif()
endif ()

add_subdirectory(modules)

if (${enable_vex_renderer})
    vex_setup_runtime(Editor)
    vex_setup_runtime(Standalone)
endif ()

get_property(_isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

if(_isMultiConfig)
    add_custom_target(copy_shaders ALL
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/raw_resources/shaders
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/shaders
            COMMENT "Copying shaders"
            VERBATIM)
else()
    add_custom_target(copy_shaders ALL
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/raw_resources/shaders
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders
            COMMENT "Copying shaders"
            VERBATIM)
endif()

add_dependencies(Editor copy_shaders)
add_dependencies(Standalone copy_shaders)

# Print build configuration
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")