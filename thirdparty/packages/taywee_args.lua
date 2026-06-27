-- taywee/args: single-header C++ argument parser
-- Check xrepo first: xrepo search args
-- If "args" appears in xrepo, replace add_requires("taywee_args") in
-- thirdparty/xmake.lua with add_requires("args") and delete this file.
package("taywee_args")
    set_kind("library", {headeronly = true})
    set_homepage("https://github.com/Taywee/args")
    set_urls("https://github.com/Taywee/args.git", {tag = "6.4.7"})

    on_install(function(package)
        -- args is a single-header library (args.hxx at the repo root)
        local dst = package:installdir("include")
        os.mkdir(dst)
        os.cp(path.join(package:sourcedir(), "args.hxx"), dst)
    end)

    on_fetch(function(package)
        return {
            includedirs = {package:installdir("include")},
            defines     = {"ARGS_NOEXCEPT"},
        }
    end)
package_end()
