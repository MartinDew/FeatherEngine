# Applied by FetchContent PATCH_COMMAND after Vex is downloaded or updated.
# VEX_SOURCE_DIR is passed via -D from the PATCH_COMMAND invocation.
# Idempotent: only writes if the target string is present.

set(VK_RHI_CPP "${VEX_SOURCE_DIR}/src/Vulkan/RHI/VkRHI.cpp")

if (EXISTS "${VK_RHI_CPP}")
    file(READ "${VK_RHI_CPP}" VK_RHI_CONTENT)
    if (VK_RHI_CONTENT MATCHES "layers.push_back\\(\"VK_LAYER_KHRONOS_synchronization2\"\\);")
        string(REPLACE
            "layers.push_back(\"VK_LAYER_KHRONOS_synchronization2\");"
            ""
            VK_RHI_CONTENT
            "${VK_RHI_CONTENT}"
        )
        file(WRITE "${VK_RHI_CPP}" "${VK_RHI_CONTENT}")
        message(STATUS "patch_vex: removed VK_LAYER_KHRONOS_synchronization2 from VkRHI.cpp")
    endif ()
endif ()
