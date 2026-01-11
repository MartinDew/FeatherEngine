FetchContent_Declare(SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG release-3.4.0 # release 3.4.x
        EXCLUDE_FROM_ALL
        FIND_PACKAGE_ARGS 3.4.0 GLOBAL
)

# if release build then use static linking
IF (${STATIC_CPP})
    set(SDL_SHARED OFF CACHE BOOL "Use shared SDL library" FORCE)
    set(SDL_STATIC ON CACHE BOOL "Use shared SDL library" FORCE)
ELSE()
    set(SDL_SHARED ON CACHE BOOL "Use shared SDL library" FORCE)
    set(SDL_STATIC OFF CACHE BOOL "Use shared SDL library" FORCE)
ENDIF()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

option(SDL_RENDERER "" ON)

set(SDL_MAIN     OFF)
set(SDL_AUDIO    OFF)
set(SDL_VIDEO    ON)
set(SDL_GPU      OFF)
set(SDL_RENDER   OFF)
set(SDL_CAMERA   OFF)
set(SDL_JOYSTICK OFF)
set(SDL_HAPTIC   OFF)
set(SDL_HIDAPI   OFF)
set(SDL_POWER    OFF)
set(SDL_SENSOR   OFF)
set(SDL_DIALOG   OFF)

set(CMAKE_DISABLE_PRECOMPILE_HEADERS ON)

FetchContent_MakeAvailable(SDL3)
RegisterThirdparty(PRIVATE SDL3::SDL3)
