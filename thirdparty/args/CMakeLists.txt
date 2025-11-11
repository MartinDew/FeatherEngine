FetchContent_Declare(args
    GIT_REPOSITORY https://github.com/Taywee/args.git
    GIT_TAG 6.4.7
    EXCLUDE_FROM_ALL
)

FetchContent_MakeAvailable(args)

get_target_property(underlying taywee::args ALIASED_TARGET)
target_compile_definitions(${underlying} INTERFACE ARGS_NOEXCEPT)

RegisterThirdparty(PRIVATE taywee::args)

