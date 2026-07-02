-- FeatherSDK.lua
-- Replaces tools/generate_export.cmake (180 lines).
--
-- Usage in an external game/plugin project's xmake.lua:
--
--   includes("/path/to/feather/tools/FeatherSDK.lua")
--
--   target("mygame_plugin")
--       set_kind("shared")
--       feather_sdk_setup("mygame_plugin", "standalone")  -- or "editor"
--       add_files("src/*.cpp")
--   target_end()
--
-- The second argument controls which feather variant to link against:
--   "editor"     → EDITOR_BUILD=1, links feather.editor.lib
--   "standalone" → EDITOR_BUILD=0, links feather.standalone.lib  (default)

local FEATHER_ROOT = path.directory(os.scriptdir())  -- one level up from tools/

function feather_sdk_setup(target_name, variant)
    variant = variant or "standalone"
    assert(variant == "editor" or variant == "standalone",
        "feather_sdk_setup: variant must be 'editor' or 'standalone', got: " .. tostring(variant))

    target(target_name)
        add_defines("EDITOR_BUILD=" .. (variant == "editor" and "1" or "0"))

        -- Engine include dirs
        add_includedirs(path.join(FEATHER_ROOT, "core"),   {public = true})
        add_includedirs(FEATHER_ROOT,                       {public = true})

        -- Public thirdparty headers that appear in engine API headers
        add_includedirs(path.join(FEATHER_ROOT, "thirdparty", "DirectXMath"), {public = true})
        add_includedirs(path.join(FEATHER_ROOT, "thirdparty", "SimpleMath"),  {public = true})

        -- Link against the engine import library on Windows
        if is_plat("windows") then
            -- Default: look for the Development (releasedbg) build
            local build_dir = path.join(FEATHER_ROOT, "build", "bin")
            add_linkdirs(build_dir)
            add_links("feather." .. variant)
        else
            -- On POSIX the executable can be linked against directly via -rdynamic
            -- Point add_linkdirs at wherever the feather binary lives
            add_linkdirs(path.join(FEATHER_ROOT, "build", "bin"))
            add_links("feather." .. variant)
        end
    target_end()
end
