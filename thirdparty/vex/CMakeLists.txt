if (APPLE) # Vex does not support metal at the moment
    return()
endif ()

FetchContent_Declare(Vex
        GIT_REPOSITORY https://github.com/Narvin-Chana/Vex.git
        GIT_TAG main
        EXCLUDE_FROM_ALL)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(VEX_ENABLE_SLANG OFF)

FetchContent_MakeAvailable(Vex)

target_compile_definitions(Vex PUBLIC
        $<$<CONFIG:Development>:VEX_DEBUG=0 VEX_DEVELOPMENT=1 VEX_SHIPPING=0>
)

RegisterThirdparty(PRIVATE Vex)