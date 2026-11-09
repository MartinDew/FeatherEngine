option("enable_resources")
    set_default(true)
    set_category("feather_modules")
    set_description("resources: resource loader, meshes, materials, shaders, textures")
    add_deps("enable_core")
    after_check("feather_module_option")
option_end()

target("resources")
    add_rules("feather.module")
    add_deps("core")
    add_packages("assimp", {public = true}) -- its Importer is part of a public partition
