-- taywee/args: single-header C++ argument parser, plus a local `import args;`
-- module wrapper (taywee_args/args.cppm).
-- If "args" ever appears in xrepo (xrepo search args), switch to it and keep only the module wrapper.
package("taywee_args")
    set_kind("library")
    set_homepage("https://github.com/Taywee/args")
    set_urls("https://github.com/Taywee/args.git", {tag = "6.4.7"})

    add_includedirs("include")
    add_defines("ARGS_NOEXCEPT")

    on_load(function(package)
        package:add("defines", "ARGS_NOEXCEPT")
    end)

    on_install(function(package)
        -- Installs the interface with its meta-info, so add_packages("taywee_args")
        -- consumers build the BMI for `import args;` and link the object from here.
        os.cp(path.join(os.scriptdir(), "taywee_args", "args.cppm"), "args.cppm")
        io.writefile("xmake.lua", [[
            target("args")
                set_kind("static")
                set_languages("c++20")
                add_defines("ARGS_NOEXCEPT", {public = true})
                add_includedirs(".")
                add_headerfiles("args.hxx")
                add_files("args.cppm", {public = true})
        ]])
        import("package.tools.xmake").install(package)
    end)
package_end()

add_requires("taywee_args 6.4.7", {system = false, alias = "taywee_args"})
