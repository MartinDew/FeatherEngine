include(FetchContent)

set(THIRDPARTIES "")

if (MSVC)
    add_compile_options("/w")
else ()
    add_compile_options("-w")
endif ()

function(RegisterThirdparty ACCESS)
    set(LOCAL_ARGV "")

    list(APPEND THIRDPARTIES ${ARGN})
    set(THIRDPARTIES ${THIRDPARTIES} CACHE INTERNAL "The list of thirdparty libraries")

    target_link_libraries(Editor ${ACCESS} ${ARGN})
    target_link_libraries(Standalone ${ACCESS} ${ARGN})
endfunction()


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_MAP_IMPORTED_CONFIG_Development "RelWithDebInfo" CACHE STRING "" FORCE)

# Shadow CMAKE_BUILD_TYPE for this scope only so FetchContent deps build as RelWithDebInfo.
# Does not touch the cache — parent scope retains "Development".
if (CMAKE_BUILD_TYPE STREQUAL "Development")
    set(CMAKE_BUILD_TYPE "RelWithDebInfo")
endif ()

set(THIRDPARTIES "" CACHE INTERNAL "The list of thirdparty libraries")

# also add subdirectories for the libraries that are directly built 
file(GLOB SUBDIR_CMAKELISTS "*/CMakeLists.txt")
foreach (CM_FILE ${SUBDIR_CMAKELISTS})
    get_filename_component(SUBDIR_NAME ${CM_FILE} DIRECTORY)
    add_subdirectory(${SUBDIR_NAME})
endforeach ()


