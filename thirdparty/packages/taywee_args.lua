-- taywee/args: single-header C++ argument parser
-- Check xrepo first: xrepo search args
-- If "args" appears in xrepo, replace add_requires("taywee_args") in
-- thirdparty/xmake.lua with add_requires("args") and delete this file.
package("taywee_args")
    set_kind("library", {headeronly = true})
    set_homepage("https://github.com/Taywee/args")
    set_urls("https://github.com/Taywee/args.git", {tag = "6.4.7"})

    on_install(function(package)
        -- CWD is the package source dir in on_install; args.hxx is at the repo root
        os.cp("args.hxx", package:installdir("include"))
        -- package:add_exported_defines("ARGS_NOEXCEPT")
        package:add("defines", "ARGS_NOEXCEPT")
    end)

    on_fetch(function(package)
        local inc = package:installdir("include")
        if not os.isfile(path.join(inc, "args.hxx")) then
            return nil
        end
        return {
            includedirs = {inc},
            defines     = {"ARGS_NOEXCEPT"},
        }
    end)
package_end()
