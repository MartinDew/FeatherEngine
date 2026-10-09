option("enable_vex_renderer")
    set_default(not is_plat("macosx"))
    set_description("Enable the vex_renderer module (off on Apple; Vex does not support Metal)")
option_end()

if (is_plat("macosx")) then
    return
end

if has_config("enable_vex_renderer") then
    -- Rules stack across a target; on_load/after_build closures don't (see
    -- xmake/helper.lua), so Vex's runtime deploy step lives in a rule instead.
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
                -- The renderer's shader include dir is <cwd>/shaders, which is where `import Vex` resolves.
                local shaders_dst = path.join(target:targetdir(), "shaders")
                os.mkdir(shaders_dst)
                os.cp(path.join(shaders_src, "*"), shaders_dst)
            end
        end)
    rule_end()

    target("vex_renderer")
        set_kind("object")
        add_rules("feather.module")
        add_files("*.cppm", {public = true})
        add_files("register_module.cpp", "vex_renderer.cpp")
        -- Produced by generate_reflection.py (see run_codegen in xmake/engine.lua); absent before the first build.
        add_files("register_vex_renderer_types.gen.cpp", {always_added = true})
        add_packages("vex", {public = true})

        -- The package ships Vex's module interface (`import Vex;`); it is compiled as part of this target.
        on_load(function(target)
            local vex = target:pkg("vex")
            if not vex then return end
            local vex_module = path.join(vex:installdir(), "modules", "Vex.cppm")
            assert(os.isfile(vex_module), "vex package has no modules/Vex.cppm; reinstall it (xmake require --force vex)")
            target:add("files", vex_module, {public = true})
        end)

        before_build(function(target)
            import("feather_codegen")
            feather_codegen.run_module_codegen(os.scriptdir())
        end)

        if is_mode("debug") then
            add_defines("VEX_DEBUG=1", "VEX_DEVELOPMENT=0", "VEX_SHIPPING=0", {public = true})
        elseif is_mode("releasedbg") then
            add_defines("VEX_DEBUG=0", "VEX_DEVELOPMENT=1", "VEX_SHIPPING=0", {public = true})
        elseif is_mode("release") then
            add_defines("VEX_DEBUG=0", "VEX_DEVELOPMENT=0", "VEX_SHIPPING=1", {public = true})
        end
    target_end()
end
