set(enable_by_default ON)
if (APPLE)
    set(enable_by_default OFF)
endif ()

feather_module(vex_renderer ${enable_by_default}
        vex_renderer.cpp
        vex_renderer.h
)

target_link_libraries(vex_renderer INTERFACE Vex)
