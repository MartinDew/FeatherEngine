include(FetchContent)

set(THIRDPARTIES "")

macro(RegisterThirdparty)
    list(APPEND THIRDPARTIES ${ARGV})
    set(THIRDPARTIES ${THIRDPARTIES} PARENT_SCOPE)
endmacro()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin CACHE PATH "Runtime output directory")

# also add subdirectories for the libraries that are directly built 
file(GLOB SUBDIR_CMAKELISTS "*/CMakeLists.txt")
foreach (CM_FILE ${SUBDIR_CMAKELISTS})
    get_filename_component(SUBDIR_NAME ${CM_FILE} DIRECTORY)
    add_subdirectory(${SUBDIR_NAME})
endforeach ()

set(THIRDPARTIES ${THIRDPARTIES} PARENT_SCOPE)
