option("enable_vex_renderer")
    set_default(not is_plat("macosx"))
    set_category("feather_modules")
    set_description("vex_renderer: Vex-based renderer (off on Apple; Vex does not support Metal)")
    add_deps("enable_rendering")
    after_check("feather_module_option")
option_end()

if (is_plat("macosx")) then
    return
end

-- Rules stack across a target; on_load/after_build closures don't (see
-- xmake/rules.lua), so Vex's runtime deploy step lives in a rule instead.
rule("vex_renderer.deploy_runtime")
    on_load(function(target)
        -- D3D12 reads D3D12SDKVersion/D3D12SDKPath from the main exe at
        -- startup, so DX12AgilitySDK.cpp must compile into the exe itself.
        if not is_plat("windows") then return end
        local vex = target:pkg("vex")
        if not vex then return end
        local agility_src = path.join(vex:installdir(), "src", "DX12", "DX12AgilitySDK.cpp")
        if os.isfile(agility_src) then
            target:add("files", agility_src)
            -- DX12AgilitySDK.cpp includes "DX12Headers.h" unqualified; Vex's
            -- on_fetch only exposes include/, not include/DX12.
            target:add("includedirs", path.join(vex:installdir(), "include", "DX12"))
            target:add("defines", "DIRECTX_AGILITY_SDK_VERSION=618")
            target:add("defines", "D3D12_AGILITY_SDK_ENABLED")
        end
    end)

    after_build(function(target)
        local vex = target:pkg("vex")
        if not vex then return end
        local tdir = target:targetdir()
        if (not is_plat("windows")) then
            tdir = path.join(tdir, "runtime")
            os.mkdir(tdir)
        end
        -- installdir(subpath) ignores subpath args, returns the package root.
        local root = vex:installdir()

        local runtime_dir = path.join(root, "runtime")
        if os.isdir(runtime_dir) then
            for _, pat in ipairs({"*.dll", "*.so*"}) do
                for _, f in ipairs(os.files(path.join(runtime_dir, pat))) do
                    os.cp(f, tdir)
                end
            end
        end

        if (is_plat("windows")) then
            -- D3D12 Agility SDK DLLs come from xrepo's directx12-agility package,
            -- not Vex's internal _deps/ (see thirdparty/xmake.lua).
            local agility = target:pkg("directx12-agility")
            if agility then
                local agility_bin = path.join(agility:installdir(), "bin")
                if os.isdir(agility_bin) then
                    local d3d12_dst = path.join(tdir, "D3D12")
                    os.mkdir(d3d12_dst)
                    os.cp(path.join(agility_bin, "D3D12Core.dll"), d3d12_dst)
                    os.cp(path.join(agility_bin, "d3d12SDKLayers.dll"), d3d12_dst)
                end
            end
        end

        local shaders_src = path.join(root, "shaders")
        if os.isdir(shaders_src) then
            -- The renderer's shader include dir is engine://shaders (next to the executable), which is where `import Vex` resolves.
            local shaders_dst = path.join(target:targetdir(), "shaders")
            os.mkdir(shaders_dst)
            os.cp(path.join(shaders_src, "*"), shaders_dst)
        end
    end)
rule_end()

target("vex_renderer")
    add_rules("feather.module")
    add_deps("rendering")
    add_packages("vex", {public = true})

    -- The package ships Vex's module interface (`import Vex;`); it is compiled as part of this target.
    -- Not asserted at load: that would also block `xmake require --force vex`, the fix for a stale install.
    on_load(function(target)
        local vex = target:pkg("vex")
        if not vex then return end
        local vex_module = path.join(vex:installdir(), "modules", "Vex.cppm")
        if os.isfile(vex_module) then
            target:add("files", vex_module, {public = true})
        end
    end)

    -- Runs before module dependency scanning, which would otherwise fail with a less helpful error.
    -- Skipped under `xmake require` (it runs config with require=false), which must stay usable to repair a stale install.
    on_config(function(target)
        import("core.base.option")
        import("feather_codegen")
        -- The renderer embeds raw_resources/shaders (raw_resources/shaders/*.slang.gen.h).
        feather_codegen.run_embedded_resources()
        if option.get("require") == false then return end
        local vex = target:pkg("vex")
        assert(not vex or os.isfile(path.join(vex:installdir(), "modules", "Vex.cppm")),
               "vex package has no modules/Vex.cppm; reinstall it (xmake require --force vex)")
    end)

    if is_mode("debug") then
        add_defines("VEX_DEBUG=1", "VEX_DEVELOPMENT=0", "VEX_SHIPPING=0", {public = true})
    elseif is_mode("releasedbg") then
        add_defines("VEX_DEBUG=0", "VEX_DEVELOPMENT=1", "VEX_SHIPPING=0", {public = true})
    elseif is_mode("release") then
        add_defines("VEX_DEBUG=0", "VEX_DEVELOPMENT=0", "VEX_SHIPPING=1", {public = true})
    end
target_end()

if has_config("enable_vex_renderer") then
    -- The deploy rule runs on the executable: it compiles DX12AgilitySDK.cpp into it and copies DLLs to its targetdir.
    -- target:pkg() only sees a target's own packages, so the rule's packages are added here too.
    target("feather")
        add_packages("vex")
        if is_plat("windows") then
            add_packages("directx12-agility")
        end
        add_rules("vex_renderer.deploy_runtime")
    target_end()
end
