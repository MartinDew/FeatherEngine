-- os.scriptdir(): the project root is this file's parent.
local FEATHER_ROOT = path.directory(os.scriptdir())

-- Applied to every feather target: shared compile flags and the
-- FEATHER_<NAME>_ENABLED define of every enabled module.
rule("feather.base")
    on_load(function (target)
        import("feather_module")
        for _, name in ipairs(feather_module.enabled_modules()) do
            target:add("defines", feather_module.macro_prefix(name) .. "_ENABLED")
        end
    end)

    -- Same flags on every target, so xmake reuses module BMIs instead of
    -- rebuilding them per target (GCC rejects mixing the two: CRC mismatch).
    on_config(function (target)
        import("feather_flags")
        feather_flags.apply(target)
    end)
rule_end()

-- A module is the target of the same name as its directory, enabled by the
-- enable_<name> option; see xmake/modules/feather_module.lua for the details.
rule("feather.module")
    add_deps("feather.base")
    on_load(function (target)
        import("feather_module")
        feather_module.load(target)
    end)

    on_config(function (target)
        import("feather_codegen")
        feather_codegen.run(target)
    end)

    before_link(function (target)
        import("feather_module")
        feather_module.strip_std_objects(target)
    end)
rule_end()

-- On the executable: links the static module libraries whole-archive.
rule("feather.link_modules")
    on_config(function (target)
        import("feather_module")
        feather_module.link_whole(target)
    end)
rule_end()

-- Copies raw_resources/shaders next to the built executable. Modules needing
-- their own post-build deploy steps should define their own rule (see
-- vex_renderer.deploy_runtime) -- rules stack across a target, closures don't.
rule("feather.deploy_shaders")
    after_build(function(target)
        -- "dir/*" copies the contents; copying the dir itself nests it inside an existing "shaders" dir
        local dst = path.join(target:targetdir(), "shaders")
        os.mkdir(dst)
        os.cp(path.join(FEATHER_ROOT, "raw_resources", "shaders", "*"), dst)
    end)
rule_end()

-- Copies flecs/sdl3's shared libraries (if built shared) next to the executable --
-- add_packages() only wires the import lib, not the runtime file itself.
rule("feather.deploy_shared_deps")
    after_build(function(target)
        for _, pkgname in ipairs({"flecs", "sdl3"}) do
            local pkg = target:pkg(pkgname)
            if pkg then
                -- installdir(subpath) ignores the arg, returns the root.
                local libdir = path.join(pkg:installdir(), target:is_plat("windows", "mingw") and "bin" or "lib")
                for _, pattern in ipairs({"*.dll", "*.so*"}) do
                    for _, f in ipairs(os.files(path.join(libdir, pattern))) do
                        local dst = path.join(target:targetdir(), path.filename(f))
                        os.tryrm(dst)
                        os.cp(f, dst, {symlink = true})
                    end
                end
            end
        end
    end)
rule_end()
