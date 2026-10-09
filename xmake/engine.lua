-- The feather executable. Every modules/<name> directory is a target of the same
-- name using the feather.module rule, linked in automatically.
local ROOT = path.directory(os.scriptdir())

target("feather")
    set_kind("binary")
    set_basename("feather")
    set_targetdir("$(builddir)/bin")

    add_files(path.join(ROOT, "modules/*.cppm"), path.join(ROOT, "modules/*.cpp"))
    add_deps("feather_core")
    add_deps("simplemath") -- object-kind deps don't propagate across a second hop
    add_packages("flecs", "assimp", "sdl3")
    -- xmake ignores a dep with no matching target (e.g. a disabled module).
    for _, dir in ipairs(os.dirs(path.join(ROOT, "modules", "*"))) do
        add_deps(path.filename(dir))
    end
    add_rules("feather.deploy_shaders", "feather.deploy_shared_deps")

    if is_plat("linux") then
        add_rpathdirs("$ORIGIN/lib", "$ORIGIN/runtime")
        -- Lets a dlopen'd project DLL resolve engine symbols at runtime.
        add_ldflags("-rdynamic", {force = true})
    end
    if is_plat("windows", "mingw") then
        add_syslinks("shell32") -- project_settings.cpp calls SHGetFolderPathA
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

-- Must come after the executable so module xmake.lua files can add packages and rules to it.
includes(path.join(ROOT, "modules", "xmake.lua"))
