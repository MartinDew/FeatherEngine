-- Realtime Math, plus a local `import rtm;` module wrapper (rtm/rtm.cppm).
-- RTM's headers define static SSE intrinsics; included once here instead of in every math
-- partition, they exist once, which is what lets the math types inline across modules.
package("rtm")
    set_kind("library")
    set_homepage("https://github.com/nfrechette/rtm")
    add_urls("https://github.com/nfrechette/rtm/archive/refs/tags/$(version).tar.gz",
             "https://github.com/nfrechette/rtm.git", {submodules = false})
    add_versions("v2.3.1", "a16fc698feca580533fa12c92fe7d1df4f341f807df7ec314274659fdfec11fb")

    add_includedirs("include")

    on_install(function(package)
        os.cp(path.join(os.scriptdir(), "rtm", "rtm.cppm"), "rtm.cppm")
        io.writefile("xmake.lua", [[
            target("rtm")
                set_kind("static")
                set_languages("c++20")
                add_includedirs("includes", {public = true})
                add_headerfiles("includes/(rtm/**.h)")
                add_files("rtm.cppm", {public = true})
        ]])
        import("package.tools.xmake").install(package)
    end)
package_end()
