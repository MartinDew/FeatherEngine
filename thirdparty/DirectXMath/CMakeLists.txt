FetchContent_Declare(DirectXMath
        GIT_REPOSITORY https://github.com/microsoft/DirectXMath.git
        GIT_TAG apr2025
        EXCLUDE_FROM_ALL)


# Add this folder as include folder

FetchContent_MakeAvailable(DirectXMath)

add_library(SAL_H INTERFACE EXCLUDE_FROM_ALL sal.h)
target_include_directories(SAL_H INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_options(DirectXMath INTERFACE
        $<$<CXX_COMPILER_ID:MSVC>:/W0>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-w>
)


RegisterThirdparty(PRIVATE SAL_H DirectXMath)

