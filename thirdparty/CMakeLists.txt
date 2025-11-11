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


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin CACHE PATH "Runtime output directory")
set(CMAKE_MAP_IMPORTED_CONFIG_Development "RelWithDebInfo" CACHE STRING "" FORCE)

# Setup so that development build matches RelWithDebInfo
if (CMAKE_BUILD_TYPE STREQUAL "Development")
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "")
endif ()

set(THIRDPARTIES "" CACHE INTERNAL "The list of thirdparty libraries")

# also add subdirectories for the libraries that are directly built 
file(GLOB SUBDIR_CMAKELISTS "*/CMakeLists.txt")
foreach (CM_FILE ${SUBDIR_CMAKELISTS})
    get_filename_component(SUBDIR_NAME ${CM_FILE} DIRECTORY)
    add_subdirectory(${SUBDIR_NAME})
endforeach ()

# set(THIRDPARTIES ${THIRDPARTIES} PARENT_SCOPE)
# target_link_libraries(Editor PRIVATE ${THIRDPARTIES})
# target_link_libraries(Standalone PRIVATE ${THIRDPARTIES})

# target_link_libraries(Editor PUBLIC SimpleMath)
# target_link_libraries(Standalone PUBLIC SimpleMath)


if (CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    set(CMAKE_BUILD_TYPE "Development" CACHE STRING "")
endif ()