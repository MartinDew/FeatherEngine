option("enable_reflection")
    set_default(true)
    set_category("feather_modules")
    set_description("reflection: Reflected, Variant, VariantArray, Callable, ClassInfo, ClassDB")
    add_deps("enable_math")
    after_check("feather_module_option")
option_end()

target("reflection")
    add_rules("feather.module")
    add_deps("math")
    -- math sits below reflection and describes its own types; its enums are registered here.
    set_values("feather.bind_dirs", "math")
