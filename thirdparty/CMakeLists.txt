include(FetchContent)

set(THIRDPARTIES "")

macro(RegisterThirdparty)
    list(APPEND THIRDPARTIES ${ARGV})
    set(THIRDPARTIES ${THIRDPARTIES} PARENT_SCOPE)
endmacro()

# also add subdirectories for the libraries that are directly built 
file(GLOB SUBDIR_CMAKELISTS "*/CMakeLists.txt")
foreach(CM_FILE ${SUBDIR_CMAKELISTS})
    get_filename_component(SUBDIR_NAME ${CM_FILE} DIRECTORY)
    add_subdirectory(${SUBDIR_NAME})
endforeach()

set(THIRDPARTIES ${THIRDPARTIES} PARENT_SCOPE)
