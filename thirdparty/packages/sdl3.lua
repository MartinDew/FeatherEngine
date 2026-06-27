-- SDL3 custom package: matches the CMake config exactly (VIDEO only, no RENDERER/GPU/etc.)
-- If xrepo gets an sdl3 package with per-subsystem config knobs, this can be removed.
-- Check first: xrepo search sdl3
package("sdl3_feather")
    set_kind("library")
    set_homepage("https://libsdl.org")
    set_urls("https://github.com/libsdl-org/SDL.git", {tag = "release-3.4.0"})
    add_deps("cmake")

    on_install(function(package)
        local static = not package:config("shared")
        import("package.tools.cmake").install(package, {
            ["SDL_SHARED"]      = static and "OFF" or "ON",
            ["SDL_STATIC"]      = static and "ON"  or "OFF",
            ["SDL_STATIC_PIC"]  = "ON",
            ["SDL_MAIN"]        = "OFF",
            ["SDL_AUDIO"]       = "OFF",
            ["SDL_VIDEO"]       = "ON",
            ["SDL_GPU"]         = "OFF",
            ["SDL_RENDER"]      = "OFF",
            ["SDL_CAMERA"]      = "OFF",
            ["SDL_JOYSTICK"]    = "OFF",
            ["SDL_HAPTIC"]      = "OFF",
            ["SDL_HIDAPI"]      = "OFF",
            ["SDL_POWER"]       = "OFF",
            ["SDL_SENSOR"]      = "OFF",
            ["SDL_DIALOG"]      = "OFF",
            ["SDL_TESTS"]       = "OFF",
        })
    end)

    -- on_load: set link names and Windows system libs for static builds.
    -- cmake auto-discover handles includedirs/linkdirs; we only need to name the libs.
    on_load(function(package)
        if package:config("shared") then
            package:add("links", "SDL3")
        else
            package:add("links", "SDL3-static")
            if package:is_plat("windows") then
                package:add("syslinks", "winmm", "imm32", "version", "setupapi")
            end
        end
    end)
package_end()
