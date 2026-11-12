option("enable_core")
    set_default(true)
    set_category("feather_modules")
    set_description("core: window, settings, shared libraries, engine context")
    add_deps("enable_reflection")
    after_check("feather_module_option")
option_end()

target("core")
    add_rules("feather.module")
    add_deps("reflection")
    -- Binds and registers the math types: ClassDB lives here, above reflection and math.
    set_values("feather.bind_dirs", "math")
    add_packages("taywee_args", {public = true})
    -- SDL3 owns process-global state: linked once into the executable unless it is shared.
    if is_plat("windows", "mingw") or is_config("module_kind", "shared") then
        add_packages("sdl3", {public = true})
    else
        add_packages("sdl3", {public = true, links = {}})
    end
    if is_plat("windows", "mingw") then
        add_syslinks("shell32") -- project_settings.cpp calls SHGetFolderPathA
    end
