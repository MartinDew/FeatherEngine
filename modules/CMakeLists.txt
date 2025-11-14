get_target_property(EDITOR_INCLUDES Editor INCLUDE_DIRECTORIES)
get_target_property(STANDALONE_INCLUDES Standalone INCLUDE_DIRECTORIES)

get_target_property(EDITOR_COMPILE_OPTIONS Editor COMPILE_OPTIONS)
get_target_property(STANDALONE_COMPILE_OPTIONS Editor INTERFACE_COMPILE_OPTIONS)

function(feather_module module_name enabled_by_default)
    option("enable_${module_name}" "Enable the ${module_name} module" ${enabled_by_default})
    if (enable_${module_name})
        add_library(${module_name} INTERFACE ${ARGN})
        target_compile_definitions(${module_name} INTERFACE ${module_name}_EXPORTS)

        target_include_directories(
                ${module_name} INTERFACE
                ${CMAKE_SOURCE_DIR}
                ${CMAKE_CURRENT_SOURCE_DIR}>
        )

        target_compile_definitions(${module_name} INTERFACE
                $<$<CONFIG:Debug>:BETA>
                $<$<CONFIG:Development>:BETA>
        )

        # Editor module
        add_library(${module_name}_editor ${FEATHER_BUILD_TYPE} ${ARGN})
        target_link_libraries(${module_name}_editor INTERFACE ${module_name} Editor PRIVATE ${THIRDPARTIES} )
        target_include_directories(${module_name}_editor PRIVATE ${EDITOR_INCLUDES})
        target_compile_options(${module_name}_editor PRIVATE ${EDITOR_COMPILE_OPTIONS} )

        # Standalone module
        add_library(${module_name}_standalone ${FEATHER_BUILD_TYPE} ${ARGN})
        target_link_libraries(${module_name}_standalone INTERFACE ${module_name} Standalone PRIVATE ${THIRDPARTIES})
        target_include_directories(${module_name}_standalone PRIVATE ${STANDALONE_INCLUDES})
        target_compile_options(${module_name}_standalone PRIVATE ${STANDALONE_COMPILE_OPTIONS})

        list(APPEND STANDALONE_MODULES ${module_name}_standalone)
        list(APPEND EDITOR_MODULES ${module_name}_editor)

        message(STATUS "Enabled module: ${module_name}")

        if (NOT ${STATIC_CPP})
            if (MSVC)
                target_link_options(${module_name}_editor PRIVATE /FORCE:UNRESOLVED)
                target_link_options(${module_name}_standalone PRIVATE /FORCE:UNRESOLVED)
            else()
                target_link_options(${module_name}_editor PRIVATE --allow-shlib-undefined)
                target_link_options(${module_name}_standalone PRIVATE --allow-shlib-undefined)
            endif()
        endif()

        set(STANDALONE_MODULES ${STANDALONE_MODULES} CACHE INTERNAL " List of enabled modules")
        set(EDITOR_MODULES ${EDITOR_MODULES} CACHE INTERNAL " List of enabled modules")
    endif ()
endfunction()

set(EDITOR_MODULES "" CACHE INTERNAL "List of enabled modules")
set(STANDALONE_MODULES "" CACHE INTERNAL "List of enabled modules")

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

file(GLOB SUBDIR_CMAKELISTS "*/CMakeLists.txt")
foreach (CM_FILE ${SUBDIR_CMAKELISTS})
    get_filename_component(SUBDIR_NAME ${CM_FILE} DIRECTORY)
    add_subdirectory(${SUBDIR_NAME})
endforeach ()

message(STATUS "Enabled EDITOR modules: ${EDITOR_MODULES}")
target_link_libraries(Editor PUBLIC ${EDITOR_MODULES})
message(STATUS "Enabled STANDALONE modules: ${STANDALONE_MODULES}")
target_link_libraries(Standalone PUBLIC ${STANDALONE_MODULES})

#set(MODULES ${MODULES} CACHE INTERNAL "List of enabled modules" PARENT_SCOPE)