option("enable_world")
    set_default(true)
    set_category("feather_modules")
    set_description("world: ECS, scenes, components, world simulation")
    add_deps("enable_rendering")
    after_check("feather_module_option")
option_end()

target("world")
    add_rules("feather.module")
    add_deps("rendering")
    -- flecs owns process-global state: linked once into the executable unless it is shared.
    if is_plat("windows", "mingw") or is_config("module_kind", "shared") then
        add_packages("flecs", {public = true})
    else
        add_packages("flecs", {public = true, links = {}})
    end
