-- DirectXMath: header-only math library
-- Includes a local sal.h shim (thirdparty/DirectXMath/sal.h) for non-MSVC platforms.
package("directxmath_feather")
    set_kind("library", {headeronly = true})
    set_homepage("https://github.com/microsoft/DirectXMath")
    set_urls("https://github.com/microsoft/DirectXMath.git", {tag = "apr2025"})

    -- Declare include dir at package level so xmake resolves it relative to
    -- installdir without any runtime guard. A custom on_fetch() that returns
    -- nil silently drops include dirs for headeronly packages.
    add_includedirs("include")

    on_install(function(package)
        local dst_inc = package:installdir("include")
        os.mkdir(dst_inc)
        -- CWD is the package source dir in on_install; headers are in Inc/
        if os.isdir("Inc") then
            os.cp(path.join("Inc", "*.h"),   dst_inc)
            os.cp(path.join("Inc", "*.inl"), dst_inc)
        end
        -- sal.h shim for non-MSVC (lives locally in the feather repo)
        local sal_src = path.join(os.projectdir(), "thirdparty", "DirectXMath", "sal.h")
        if os.isfile(sal_src) then
            os.cp(sal_src, dst_inc)
        end
    end)
package_end()
