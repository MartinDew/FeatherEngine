function(feather_module module_name enabled_by_default)
    option(ENABLE_${module_name} "Enable the ${module_name} module" ${enabled_by_default})
    if (ENABLE_${module_name})
        add_library(${module_name} INTERFACE ${ARGN})
        target_compile_definitions(${module_name} INTERFACE ${module_name}_EXPORTS)

        target_include_directories(
                ${module_name} INTERFACE
                ${CMAKE_SOURCE_DIR}/core
                ${CMAKE_CURRENT_SOURCE_DIR}>
        )

        target_compile_definitions(${module_name} INTERFACE
                $<$<CONFIG:Debug>:BETA>
                $<$<CONFIG:Development>:BETA>
        )

        # Editor module
        add_library(${module_name}_editor ${FEATHER_BUILD_TYPE} ${module_name})
        target_compile_definitions(${module_name}_editor PRIVATE EDITOR_BUILD=1)
        target_link_libraries(${module_name}_editor PRIVATE ${module_name} Editor)

        # Standalone module
        add_library(${module_name}_standalone ${FEATHER_BUILD_TYPE} ${module_name})
        target_compile_definitions(${module_name}_standalone PRIVATE EDITOR_BUILD=0)
        target_link_libraries(${module_name}_standalone PRIVATE ${module_name} Standalone)

        list(APPEND STANDALONE_MODULES ${module_name}_standalone)
        list(APPEND EDITOR_MODULES ${module_name}_editor)

        message(STATUS "Enabled module: ${module_name}")

        set(STANDALONE_MODULES ${STANDALONE_MODULES} CACHE INTERNAL " List of enabled modules")
        set(EDITOR_MODULES ${EDITOR_MODULES} CACHE INTERNAL " List of enabled modules")
    endif ()
endfunction()

set(EDITOR_MODULES "" CACHE INTERNAL "List of enabled modules")
set(STANDALONE_MODULES "" CACHE INTERNAL "List of enabled modules")

file(GLOB SUBDIR_CMAKELISTS "*/CMakeLists.txt")
foreach (CM_FILE ${SUBDIR_CMAKELISTS})
    get_filename_component(SUBDIR_NAME ${CM_FILE} DIRECTORY)
    add_subdirectory(${SUBDIR_NAME})
endforeach ()

message(STATUS "Enabled EDITOR modules: ${EDITOR_MODULES}")
message(STATUS "Enabled STANDALONE modules: ${EDITOR_MODULES}")

#set(MODULES ${MODULES} CACHE INTERNAL "List of enabled modules" PARENT_SCOPE)