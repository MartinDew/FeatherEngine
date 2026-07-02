---- taywee/args: single-header C++ argument parser
---- Check xrepo first: xrepo search args
---- If "args" appears in xrepo, replace add_requires("taywee_args") in
---- thirdparty/xmake.lua with add_requires("args") and delete this file.
package("taywee_args")
    set_kind("library", {headeronly = true})
    set_homepage("https://github.com/Taywee/args")
    set_urls("https://github.com/Taywee/args.git", {tag = "6.4.7"})

    add_includedirs("include")
    add_defines("ARGS_NOEXCEPT")

    on_load(function(package)
        -- Load any necessary configurations or dependencies
        package:add("defines", "ARGS_NOEXCEPT")
    end)

    on_install(function(package)
        -- CWD is the package source dir in on_install; args.hxx is at the repo root
        os.cp("args.hxx", package:installdir("include"))
    end)
package_end()

-- taywee/args (single-header, ARGS_NOEXCEPT)
add_requires("taywee_args 6.4.7", {
    system = false,
    alias  = "taywee_args",
})