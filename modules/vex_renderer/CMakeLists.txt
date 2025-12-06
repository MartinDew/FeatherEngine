set(enable_by_default ON)
if (APPLE)
    set(enable_by_default OFF)
endif ()

feather_module(vex_renderer ${enable_by_default}
        register_module.cpp
        vex_renderer.cpp

        register_module.h
        vex_renderer.h
)

if (enable_vex_renderer)
    target_link_libraries(vex_renderer_editor PUBLIC Vex)
    target_link_libraries(vex_renderer_standalone PUBLIC Vex)
endif ()
