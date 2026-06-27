-- DirectXMath: header-only math library
-- Includes a local sal.h shim (thirdparty/DirectXMath/sal.h) for non-MSVC platforms.
package("directxmath_feather")
    set_kind("library", {headeronly = true})
    set_homepage("https://github.com/microsoft/DirectXMath")
    set_urls("https://github.com/microsoft/DirectXMath.git", {tag = "apr2025"})

    on_install(function(package)
        local dst_inc = package:installdir("include")
        os.mkdir(dst_inc)
        -- DirectXMath headers are in Inc/ at the repo root
        local inc_dir = path.join(package:sourcedir(), "Inc")
        if os.isdir(inc_dir) then
            os.cp(path.join(inc_dir, "*.h"), dst_inc)
            os.cp(path.join(inc_dir, "*.inl"), dst_inc)
        end
        -- sal.h shim for non-MSVC (lives locally in the feather repo)
        local sal_src = path.join(os.projectdir(), "thirdparty", "DirectXMath", "sal.h")
        if os.isfile(sal_src) then
            os.cp(sal_src, dst_inc)
        end
    end)

    on_fetch(function(package)
        return {
            includedirs = {package:installdir("include")},
        }
    end)
package_end()
