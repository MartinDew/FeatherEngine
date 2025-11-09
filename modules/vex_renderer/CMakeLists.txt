feather_module(vex_renderer ON
        vex_renderer.cpp
        vex_renderer.h
)

target_link_libraries(vex_renderer INTERFACE Vex)
