option("enable_framework")
    set_default(true)
    set_category("feather_modules")
    set_description("framework: assertions, strings, containers, delegates, allocator")
    after_check("feather_module_option")
option_end()

target("framework")
    add_rules("feather.module")
