-- taywee/args: single-header C++ argument parser, consumed as a header unit
-- (`import <args.hxx>;`); xmake builds the header unit in each consumer.
-- If "args" ever appears in xrepo (xrepo search args), switch to it.
package("taywee_args")
    set_kind("library", {headeronly = true})
    set_homepage("https://github.com/Taywee/args")
    set_urls("https://github.com/Taywee/args.git", {tag = "6.4.7"})

    -- Public so the header unit is built with the same define in every consumer.
    add_defines("ARGS_NOEXCEPT")

    on_install(function(package)
        os.cp("args.hxx", package:installdir("include"))
    end)
package_end()

add_requires("taywee_args 6.4.7", {system = false, alias = "taywee_args"})
