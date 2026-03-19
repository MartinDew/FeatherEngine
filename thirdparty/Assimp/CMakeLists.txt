FetchContent_Declare(Assimp
        GIT_REPOSITORY https://github.com/assimp/assimp.git
        GIT_TAG v6.0.4
        EXCLUDE_FROM_ALL
)

set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF)
set(ASSIMP_BUILD_TESTS OFF)

FetchContent_MakeAvailable(Assimp)

RegisterThirdparty(PRIVATE Assimp)