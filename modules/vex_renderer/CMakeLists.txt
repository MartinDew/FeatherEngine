set(enable_by_default ON)
if (APPLE)
    set(enable_by_default OFF)
endif ()

feather_module(vex_renderer ${enable_by_default}
        vex_renderer.cpp
        vex_renderer.h
)

if(enable_vex_renderer)
    target_link_libraries(vex_renderer_editor INTERFACE Vex)
    target_link_libraries(vex_renderer_standalone INTERFACE Vex)
endif()
