if (APPLE) # Vex does not support metal at the moment
    return()
endif ()

FetchContent_Declare(Vex
        GIT_REPOSITORY https://github.com/Narvin-Chana/Vex.git
        GIT_TAG main
        EXCLUDE_FROM_ALL
        PATCH_COMMAND ${CMAKE_COMMAND}
            -DVEX_SOURCE_DIR=<SOURCE_DIR>
            -P "${CMAKE_CURRENT_LIST_DIR}/patch_vex.cmake"
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(VEX_ENABLE_SLANG ON)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

FetchContent_MakeAvailable(Vex)

target_compile_definitions(Vex PUBLIC
        $<$<CONFIG:Development>:VEX_DEBUG=0 VEX_DEVELOPMENT=1 VEX_SHIPPING=0>
)

target_include_directories(Editor PRIVATE ${vex_SOURCE_DIR}/examples/imgui)
# Not in release
target_include_directories(Standalone PRIVATE
        $<$<CONFIG:Debug>:${vex_SOURCE_DIR}/examples/imgui>
        $<$<CONFIG:Development>:${vex_SOURCE_DIR}/examples/imgui>
)