include(FetchContent)

set(THIRDPARTIES "")

macro(ConfigureTarget)
    get_target_property(OG ${ARGV0} ALIASED_TARGET)
    if (OG)
        set_target_properties(${OG} PROPERTIES MAP_IMPORTED_CONFIG_RELWITHDEBINFO "Development")
        # suppress all warnings
        if (MSVC)
            target_compile_options(${OG} INTERFACE "/w")
        else ()
            target_compile_options(${OG} INTERFACE "-w")
        endif ()
        list(APPEND LOCAL_ARGV ${OG})
    else ()
        set_target_properties(${TP} PROPERTIES MAP_IMPORTED_CONFIG_RELWITHDEBINFO "Development")
        # suppress all warnings
        if (MSVC)
            target_compile_options(${TP} INTERFACE "/w")
        else ()
            target_compile_options(${TP} INTERFACE "-w")
        endif ()
        list(APPEND LOCAL_ARGV ${TP})
    endif ()
endmacro()

macro(RegisterThirdparty)
    set(LOCAL_ARGV "")

    foreach (TP ${ARGV})
        # Try and extract real target name if alias
        ConfigureTarget(${TP})
    endforeach ()


    list(APPEND THIRDPARTIES ${ARGV})
    set(THIRDPARTIES ${THIRDPARTIES} PARENT_SCOPE)
endmacro()


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin CACHE PATH "Runtime output directory")
# set(CMAKE_MAP_IMPORTED_CONFIG_Development "RelWithDebInfo" CACHE STRING "" FORCE)

# Setup so that development build matches RelWithDebInfo
if (CMAKE_BUILD_TYPE STREQUAL "Development")
    set(CMAKE_BUILD_TYPE "RelWithDebInfo")
endif ()

# also add subdirectories for the libraries that are directly built 
file(GLOB SUBDIR_CMAKELISTS "*/CMakeLists.txt")
foreach (CM_FILE ${SUBDIR_CMAKELISTS})
    get_filename_component(SUBDIR_NAME ${CM_FILE} DIRECTORY)
    add_subdirectory(${SUBDIR_NAME})
endforeach ()

set(THIRDPARTIES ${THIRDPARTIES} PARENT_SCOPE)
