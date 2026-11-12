option("enable_math")
    set_default(true)
    set_category("feather_modules")
    set_description("math: vectors, matrices, quaternions, colors, transforms")
    add_deps("enable_framework")
    after_check("feather_module_option")
option_end()

target("math")
    add_rules("feather.module")
    add_deps("framework")
    add_packages("rtm", {public = true})
    -- Below reflection: its types are bound and registered by the core module (where ClassDB lives).
    set_values("feather.external_bind", true)
