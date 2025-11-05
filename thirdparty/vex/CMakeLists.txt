FetchContent_Declare(Vex
    GIT_REPOSITORY https://github.com/Narvin-Chana/Vex.git
    GIT_TAG 25653d0a3d0796ff395c25b32296f5c1b84389e4
    EXCLUDE_FROM_ALL)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(VEX_ENABLE_SLANG ON)

FetchContent_MakeAvailable(Vex)
RegisterThirdparty(Vex)