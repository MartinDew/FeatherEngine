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

if (VEX_ENABLE_DX12)
    target_sources(
            vex_renderer_editor
            PRIVATE
            ${IMGUI_SRC_DIR}/backends/imgui_impl_dx12.cpp
    )

    target_sources(
            vex_renderer_standalone
            PRIVATE
            $<$<CONFIG:Debug>:${IMGUI_SRC_DIR}/backends/imgui_impl_dx12.cpp>
            $<$<CONFIG:Development>:${IMGUI_SRC_DIR}/backends/imgui_impl_dx12.cpp>
    )
endif ()

if (VEX_ENABLE_VULKAN)
    target_sources(
            vex_renderer_editor
            PRIVATE
            ${IMGUI_SRC_DIR}/backends/imgui_impl_vulkan.cpp
    )

    target_sources(
            vex_renderer_standalone
            PRIVATE
            $<$<CONFIG:Debug>:${IMGUI_SRC_DIR}/backends/imgui_impl_vulkan.cpp>
            $<$<CONFIG:Development>:${IMGUI_SRC_DIR}/backends/imgui_impl_vulkan.cpp>
    )
endif ()

target_link_libraries(
        vex_renderer_editor
        PRIVATE
        ImGui
)

target_link_libraries(
        vex_renderer_standalone
        PRIVATE
        $<$<CONFIG:Debug>:ImGui>
        $<$<CONFIG:Development>:ImGui>
)

if (enable_vex_renderer)
    target_link_libraries(vex_renderer_editor PUBLIC Vex ImGui)
    target_link_libraries(vex_renderer_standalone PUBLIC Vex ImGui)
endif ()
