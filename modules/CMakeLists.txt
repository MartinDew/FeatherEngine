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
                ${CMAKE_CURRENT_SOURCE_DIR}
        )

        target_compile_definitions(${module_name} INTERFACE
                $<$<CONFIG:Debug>:BETA>
                $<$<CONFIG:Development>:BETA>
        )

        # Editor module
        add_library(${module_name}_editor ${FEATHER_BUILD_TYPE} ${ARGN})
        target_link_libraries(${module_name}_editor INTERFACE ${module_name} )
        target_link_libraries(${module_name}_editor PUBLIC ${THIRDPARTIES} )
        
         target_include_directories(${module_name}_editor PRIVATE
            $<TARGET_PROPERTY:Editor,INCLUDE_DIRECTORIES>
        )
        target_compile_options(${module_name}_editor PRIVATE
            $<TARGET_PROPERTY:Editor,COMPILE_OPTIONS>
        )
        target_compile_definitions(${module_name}_editor PUBLIC
            EDITOR_BUILD=1
            $<$<CONFIG:Debug>:BETA>
            $<$<CONFIG:Development>:BETA>
        )

        # Standalone module
        add_library(${module_name}_standalone ${FEATHER_BUILD_TYPE} ${ARGN})
        
        target_link_libraries(${module_name}_standalone INTERFACE ${module_name} )
        target_link_libraries(${module_name}_standalone PRIVATE ${THIRDPARTIES})
        
        target_include_directories(${module_name}_standalone PRIVATE
            $<TARGET_PROPERTY:Standalone,INCLUDE_DIRECTORIES>
        )
        target_compile_options(${module_name}_standalone PRIVATE
            $<TARGET_PROPERTY:Standalone,COMPILE_OPTIONS>
        )
        target_compile_definitions(${module_name}_standalone PUBLIC
            EDITOR_BUILD=0
            $<$<CONFIG:Debug>:BETA>
            $<$<CONFIG:Development>:BETA>
        )

        # ===== Handle Dynamic Library Linking =====
        if(NOT STATIC_CPP)  # If building shared libraries
            if(MSVC)
                # Windows: Allow unresolved symbols (will be resolved from .exe)
                target_link_options(${module_name}_editor PRIVATE /FORCE:UNRESOLVED)
                target_link_options(${module_name}_standalone PRIVATE /FORCE:UNRESOLVED)
            else()
                # Linux/macOS: Allow undefined symbols in shared libraries
                target_link_options(${module_name}_editor PRIVATE -Wl,--allow-shlib-undefined)
                target_link_options(${module_name}_standalone PRIVATE -Wl,--allow-shlib-undefined)
            endif()
        endif()

        # Add to module lists
        list(APPEND EDITOR_MODULES ${module_name}_editor)
        list(APPEND STANDALONE_MODULES ${module_name}_standalone)
        
        set(EDITOR_MODULES ${EDITOR_MODULES} CACHE INTERNAL "List of enabled editor modules")
        set(STANDALONE_MODULES ${STANDALONE_MODULES} CACHE INTERNAL "List of enabled standalone modules")
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

# Link modules to main targets
message(STATUS "Linking EDITOR modules: ${EDITOR_MODULES}")
if(EDITOR_MODULES)
    target_link_libraries(Editor PUBLIC ${EDITOR_MODULES})
endif()

message(STATUS "Linking STANDALONE modules: ${STANDALONE_MODULES}")
if(STANDALONE_MODULES)
    target_link_libraries(Standalone PUBLIC ${STANDALONE_MODULES})
endif()