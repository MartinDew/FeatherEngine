-- The feather executable: the engine entry point and main loop, plus every enabled optional
-- module under modules/, registered through feather.modules (modules.cpp).
target("feather")
    set_kind("binary")
    set_default(true)
    set_basename("feather")
    set_targetdir("$(builddir)/bin")
    add_rules("feather.base", "feather.link_modules")

    add_files("*.cppm", "*.cpp")
    add_deps("world")
    -- Every enabled module under modules/; a disabled dep would still be linked, so it is never added.
    on_load(function (target)
        import("feather_module")
        for _, dir in ipairs(os.dirs(path.join(os.projectdir(), "modules", "*"))) do
            local name = path.filename(dir)
            if os.isfile(path.join(dir, "xmake.lua")) and feather_module.enabled(name) then
                target:add("deps", name)
            end
        end
    end)
    -- Linked here, not in the modules: one copy of their process-global state.
    add_packages("flecs", "assimp", "sdl3")
    if is_config("module_kind", "shared") then
        add_files(path.join(os.projectdir(), "framework", "alloc_overrides.cpp"))
    end
    add_rules("feather.deploy_shaders", "feather.deploy_shared_deps")

    if is_plat("linux") then
        add_rpathdirs("$ORIGIN", "$ORIGIN/lib", "$ORIGIN/runtime")
        -- Lets a dlopen'd project DLL resolve engine symbols at runtime.
        add_ldflags("-rdynamic", {force = true})
    end
    if is_plat("mingw") then
        add_syslinks("stdc++exp")
        add_ldflags("-static-libgcc", "-static-libstdc++",
            "-Wl,-Bstatic,--whole-archive", "-lwinpthread", "-Wl,--no-whole-archive,-Bdynamic",
            -- GNU ld needs --out-implib explicitly (link.exe does this automatically).
            "-Wl,--out-implib,$(builddir)/bin/libfeather.a",
            {force = true})
    end
target_end()
