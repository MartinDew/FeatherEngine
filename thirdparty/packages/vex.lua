package("vex")
    set_kind("library")
    set_homepage("https://github.com/Narvin-Chana/Vex")
    set_urls("https://github.com/Narvin-Chana/Vex.git", {branch = "main"})

    -- D3D12 Agility SDK version embedded in Vex (VexDX12.cmake: DX_AGILITY_VERSION = "618")
    local AGILITY_VERSION = "618"

    on_install(function(package)
        -- CWD is the package source dir in on_install
        local installdir = package:installdir()
        -- cmake build dir lives inside installdir so it is cleaned up with the package
        local builddir = path.join(installdir, ".cmake_build")

        -- ---- Patch: remove invalid Vulkan validation layer ---------------
        local vkrhi = path.join("src", "Vulkan", "RHI", "VkRHI.cpp")
        if os.isfile(vkrhi) then
            local content = io.readfile(vkrhi)
            local patched, n = content:gsub(
                'layers%.push_back%("VK_LAYER_KHRONOS_synchronization2"%);[^\n]*\n?', "")
            if n > 0 then
                cprint("${cyan}[vex]${reset} Patched VkRHI.cpp: removed VK_LAYER_KHRONOS_synchronization2")
                io.writefile(vkrhi, patched)
            end
        end

        -- ---- cmake configure + build + install ---------------------------
        local build_type = package:debug() and "Debug" or "Release"
        os.vrunv("cmake", {
            "-S", ".",
            "-B", builddir,
            "-DCMAKE_BUILD_TYPE="     .. build_type,
            "-DCMAKE_INSTALL_PREFIX=" .. installdir,
            "-DVEX_ENABLE_SLANG=ON",
            "-DVEX_BUILD_EXAMPLES=OFF",
            "-DVEX_BUILD_TESTS=OFF",
            "-DVEX_BUILD_TOOLS=OFF",
            "-DCMAKE_WINDOWS_EXPORT_ALL_SYMBOLS=ON",
        })
        os.vrunv("cmake", {"--build",   builddir, "--config", build_type})
        os.vrunv("cmake", {"--install", builddir, "--config", build_type})

        -- ---- Collect runtime artifacts from cmake's _deps ----------------
        local deps = path.join(builddir, "_deps")

        -- Slang DLLs → runtime/
        local slang_bin = path.join(deps, "slang-src", "bin")
        if os.isdir(slang_bin) then
            os.cp(path.join(slang_bin, "*.dll"), package:installdir("runtime"))
        end

        -- DXC DLLs (x64) → runtime/
        local dxc_bin = path.join(deps, "dxc-src", "bin", "x64")
        if os.isdir(dxc_bin) then
            os.cp(path.join(dxc_bin, "*.dll"), package:installdir("runtime"))
        end

        -- WinPIX runtime (x64) → runtime/
        local pix_bin = path.join(deps, "PixEvents", "bin", "x64")
        if os.isdir(pix_bin) then
            os.cp(path.join(pix_bin, "WinPixEventRuntime.dll"), package:installdir("runtime"))
        end

        -- D3D12 Agility SDK DLLs (x64) → D3D12/
        for _, agility_dir in ipairs(os.dirs(path.join(deps, "DirectX-AgilitySDK-*"))) do
            local x64_dir = path.join(agility_dir, "build", "native", "bin", "x64")
            if os.isdir(x64_dir) then
                os.cp(path.join(x64_dir, "D3D12Core.dll"),       package:installdir("D3D12"))
                os.cp(path.join(x64_dir, "d3d12SDKLayers.dll"),  package:installdir("D3D12"))
            end
        end

        -- Vex shaders → shaders/
        if os.isdir("shaders") then
            os.cp(path.join("shaders", "*"), package:installdir("shaders"))
        end

        -- DX12AgilitySDK.cpp → src/DX12/  (must be compiled into the host executable)
        local agility_src = path.join("src", "DX12", "DX12AgilitySDK.cpp")
        if os.isfile(agility_src) then
            local dst = package:installdir("src", "DX12")
            os.mkdir(dst)
            os.cp(agility_src, dst)
        end

        -- Import libs for slang, dxc, and PIX (cmake installs DLLs but not their .lib stubs)
        local libdir = package:installdir("lib")
        local slang_lib = path.join(deps, "slang-src", "lib", "slang.lib")
        if os.isfile(slang_lib) then os.cp(slang_lib, libdir) end
        local dxc_lib = path.join(deps, "dxc-src", "lib", "x64", "dxcompiler.lib")
        if os.isfile(dxc_lib) then os.cp(dxc_lib, libdir) end
        local pix_lib = path.join(deps, "PixEvents", "bin", "x64", "WinPixEventRuntime.lib")
        if os.isfile(pix_lib) then os.cp(pix_lib, libdir) end
    end)

    on_fetch(function(package)
        local inc = package:installdir("include")
        -- Guard: if the key header isn't present, on_install hasn't run yet
        if not os.isfile(path.join(inc, "Vex.h")) then
            return nil
        end
        local libdir = package:installdir("lib")
        return {
            -- magic_enum is double-nested (include/magic_enum/magic_enum/magic_enum.hpp);
            -- adding include/magic_enum as a secondary root makes the #include work.
            -- directx/dxc/slang subdirs host their respective public headers.
            includedirs = {
                inc,
                path.join(inc, "magic_enum"),
                path.join(inc, "directx"),
                path.join(inc, "dxc"),
                path.join(inc, "slang"),
            },
            linkdirs = {libdir},
            -- Vex.lib + import libs for slang/dxc/PIX DLLs (copied to lib/ during on_install)
            links    = {"Vex", "slang", "dxcompiler", "WinPixEventRuntime"},
            -- Windows SDK graphics libs
            syslinks = {"d3d12", "dxgi", "dxguid"},
            defines  = {
                "VEX_AGILITY_SDK_VERSION=" .. AGILITY_VERSION,
                "VEX_DX12=1",
                "VEX_VULKAN=0",
                "VEX_SLANG=1",
                "VEX_SHADER_COMPILER=1",
                "VEX_DXC=1",
            },
        }
    end)
package_end()
