-- os.scriptdir(), not os.projectdir(): the latter would resolve to a
-- CONSUMER's repo if this file were ever includes()'d cross-repo.
local FEATHER_ROOT = path.directory(os.scriptdir())

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

-- Copies flecs/sdl3's shared .dll next to feather.exe on windows/mingw --
-- add_packages() only wires the import lib, not the runtime file itself.
rule("feather.deploy_shared_deps")
    after_build(function(target)
        if not target:is_plat("windows", "mingw") then return end
        for _, pkgname in ipairs({"flecs", "sdl3"}) do
            local pkg = target:pkg(pkgname)
            if pkg then
                -- installdir(subpath) ignores the arg, returns the root.
                local bindir = path.join(pkg:installdir(), "bin")
                if os.isdir(bindir) then
                    for _, f in ipairs(os.files(path.join(bindir, "*.dll"))) do
                        os.cp(f, target:targetdir())
                    end
                end
            end
        end
    end)
rule_end()

-- A feather module is a library linked into the executable automatically
-- (see xmake/engine.lua). Its sources live next to its xmake.lua.
rule("feather.module")
    on_load(function (target)
        local kind = target:kind()
        if kind ~= "static" and kind ~= "shared" and kind ~= "object" then
            raise("A feather module can only be a library type. Target '%s' is a '%s'.", target:name(), kind)
        end

        target:set("group", "modules")

        target:add("defines", target:name() .. "_ENABLED", {public = true})
        target:add("includedirs", target:scriptdir())
        target:add("deps", "feather_core")
    end)
rule_end()
