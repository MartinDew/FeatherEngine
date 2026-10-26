option("use_lto")
    set_default(false)
    set_description("Enable link-time optimization (ThinLTO with LLVM/Clang, full LTO with MSVC)")
option_end()

option("static_cpp")
    set_default(not is_plat("macosx"))
    set_description("Link the C++ standard library statically")
option_end()

option("static_deps")
    set_default(true)
    set_description("Build all third-party dependencies as static libraries")
option_end()

option("production")
    set_default(false)
    set_description("Enable production flags (implies use_lto, static_cpp, static_deps)")
option_end()

option("enable_sanitizers")
    set_default(false)
    set_description("Enable AddressSanitizer + UBSan (debug mode + LLVM/Clang only)")
option_end()

option("enable_clang_tidy")
    set_default(false)
    set_description("Run clang-tidy during compilation (requires compile_commands.json)")
option_end()

option("editor_build")
    set_default(true)
    set_description("Compile with editor support (EDITOR_BUILD=1); off builds a shipping-only binary (EDITOR_BUILD=0)")
option_end()

option("double_precision")
    set_default(false)
    set_description("Use double precision for real_t and the default vector/quaternion types (FEATHER_DOUBLE_PRECISION)")
option_end()

