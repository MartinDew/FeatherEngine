FetchContent_Declare(
    imgui_sources
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.92.8-docking
)

FetchContent_MakeAvailable(imgui_sources)

set(IMGUI_SRC_DIR ${imgui_sources_SOURCE_DIR})
set(IMGUI_SOURCES
        ${IMGUI_SRC_DIR}/imgui.cpp
        ${IMGUI_SRC_DIR}/imgui_demo.cpp
        ${IMGUI_SRC_DIR}/imgui_draw.cpp
        ${IMGUI_SRC_DIR}/imgui_tables.cpp
        ${IMGUI_SRC_DIR}/imgui_widgets.cpp
)

set(IMGUI_BACKEND_SOURCES
        ${IMGUI_SRC_DIR}/backends/imgui_impl_sdl3.cpp
        ${IMGUI_SRC_DIR}/backends/imgui_impl_vulkan.cpp
        ${IMGUI_SRC_DIR}/backends/imgui_impl_dx12.cpp
        ${IMGUI_SRC_DIR}/misc/cpp/imgui_stdlib.cpp
)

find_package(Vulkan REQUIRED)

add_library(ImGui STATIC ${IMGUI_SOURCES} ${IMGUI_BACKEND_SOURCES})
target_include_directories(ImGui PUBLIC
        ${IMGUI_SRC_DIR}
        ${IMGUI_SRC_DIR}/backends
        ${IMGUI_SRC_DIR}/misc/cpp
)
target_link_libraries(ImGui PRIVATE
        SDL3::SDL3
        Vulkan::Vulkan
)

RegisterThirdparty(PUBLIC ImGui)