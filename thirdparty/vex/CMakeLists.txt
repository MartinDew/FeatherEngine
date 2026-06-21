if (APPLE) # Vex does not support metal at the moment
    return()
endif ()

FetchContent_Declare(Vex
        GIT_REPOSITORY https://github.com/Narvin-Chana/Vex.git
        GIT_TAG main
        EXCLUDE_FROM_ALL)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(VEX_ENABLE_SLANG ON)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

FetchContent_MakeAvailable(Vex)

# Patch VkRHI.cpp to remove the non-existent VK_LAYER_KHRONOS_synchronization2 validation layer
set(VK_RHI_CPP "${vex_SOURCE_DIR}/src/Vulkan/RHI/VkRHI.cpp")
if (EXISTS "${VK_RHI_CPP}")
    file(READ "${VK_RHI_CPP}" VK_RHI_CONTENT)
    if (VK_RHI_CONTENT MATCHES "layers.push_back\\(\"VK_LAYER_KHRONOS_synchronization2\"\\);")
        message(STATUS "Patching VkRHI.cpp to remove VK_LAYER_KHRONOS_synchronization2 layer")
        string(REPLACE "layers.push_back(\"VK_LAYER_KHRONOS_synchronization2\");" "" VK_RHI_CONTENT "${VK_RHI_CONTENT}")
        file(WRITE "${VK_RHI_CPP}" "${VK_RHI_CONTENT}")
    endif ()
endif ()

target_compile_definitions(Vex PUBLIC
        $<$<CONFIG:Development>:VEX_DEBUG=0 VEX_DEVELOPMENT=1 VEX_SHIPPING=0>
)