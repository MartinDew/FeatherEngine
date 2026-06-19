include(FetchContent)

FetchContent_Declare(
        flecs
        GIT_REPOSITORY https://github.com/SanderMertens/flecs.git
        GIT_TAG v4.1.5
)

FetchContent_MakeAvailable(flecs)
RegisterThirdparty(PRIVATE flecs::flecs_static)