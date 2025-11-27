set(enable_by_default ON)
if (APPLE)
    set(enable_by_default OFF)
endif ()

set(VEX_ENABLE_SLANG OFF CACHE BOOL "Enable Slang support in Vex")

feather_module(vex_renderer ${enable_by_default}
        vex_renderer.cpp
        vex_renderer.h
        ${CMAKE_SOURCE_DIR}/internal_resources/example_cube.hlsl
)

if(enable_vex_renderer)
    target_link_libraries(vex_renderer_editor PUBLIC Vex)
    target_link_libraries(vex_renderer_standalone PUBLIC Vex)
endif()
