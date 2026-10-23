-- DirectXMath (header-only) with a local sal.h shim for non-MSVC platforms.
package("directxmath_feather")
    set_kind("library", {headeronly = true})
    set_homepage("https://github.com/microsoft/DirectXMath")
    set_urls("https://github.com/microsoft/DirectXMath.git", {tag = "apr2025"})

    -- Declared here so xmake resolves it from installdir without an on_fetch() override.
    add_includedirs("include")

    on_install(function(package)
        local dst_inc = package:installdir("include")
        os.mkdir(dst_inc)
        if os.isdir("Inc") then
            os.cp(path.join("Inc", "*.h"),   dst_inc)
            os.cp(path.join("Inc", "*.inl"), dst_inc)
        end
        -- os.scriptdir() is thirdparty/packages/, go up two levels to the
        -- engine root. Not os.projectdir(): a downstream consumer's build
        -- would resolve that to the wrong repo.
        -- Never on MSVC: the shim would shadow the real <sal.h>, and the STL's own headers
        -- (e.g. <execution>, built as part of `import std;`) need annotations it lacks.
        if not package:is_plat("windows") then
            local sal_src = path.join(path.directory(path.directory(os.scriptdir())), "thirdparty", "DirectXMath", "sal.h")
            if os.isfile(sal_src) then
                os.cp(sal_src, dst_inc)
            end
        end
    end)
package_end()
