-- FeatherPluginSDK: builds Feather extensions written in C, C++ or C# against a
-- designated API description file, with no engine checkout involved.
--
-- Every language goes through the same door -- the engine's C bindings -- so
-- none of them needs the engine's headers, its compile flags or its reflection
-- codegen:
--
--   * The engine parsed its own headers once and published the result as
--     feather_api.json (see the `export-api` task). A plugin turns that JSON
--     into C headers, C++ wrappers or C# sources with generators that link no
--     Clang and need no engine source.
--   * mrbind also emits C++ glue that calls the engine. It is deliberately
--     never compiled here: the engine already has that glue compiled into its
--     own binary. A plugin's feather_* imports stay undefined and bind to the
--     engine process when it dlopens the plugin.
--
-- Vendor this file, feather_plugin_rules.lua, modules/, packages/ and the
-- feather_<lang>/ directory for each language you build, alongside an api/ file
-- -- the same way a Godot project vendors nothing but its .gdextension.
-- See tools/templates/plugin_{c,cpp,cs}_template/.
--
-- Usage (in the plugin's xmake.lua):
--
--     includes("sdk/FeatherPluginSDK.lua")
--     feather_plugin_sdk_init()
--
--     target("my_plugin")
--         add_rules("feather.plugin.c")     -- or .cpp / .cs
--         add_files("src/*.c")
--         -- set_values("feather.api_json", "api/feather_api.json")  -- the default
--
-- The rules live in feather_plugin_rules.lua; the work they call lives in
-- modules/feather_plugin_bindings.lua (a module, not functions here: a function
-- defined in an includes()'d file like this one keeps the description sandbox as
-- its environment even when called from a rule script, and that sandbox has no
-- assert(), import() or io).

-- This file's own directory, captured while it is being included -- inside a
-- function called from the consumer's xmake.lua, os.scriptdir() would resolve to
-- the CONSUMER's directory instead.
local SDK_DIR = os.scriptdir()

-- The C++ half of the SDK (wrapper generator, headers, math sources) is optional.
-- A C or C# plugin vendors none of it and must not be made to build a generator it never runs, nor fetch DirectXMath.
local HAVE_CPP_SDK = os.isdir(path.join(SDK_DIR, "feather_cpp", "gen_cpp"))

-- Call once, before any feather plugin target.
function feather_plugin_sdk_init()
    add_moduledirs(path.join(SDK_DIR, "modules"))
    includes(path.join(SDK_DIR, "packages", "mrbind_generators.lua"))
    if HAVE_CPP_SDK then
        -- Header-only, and the C++ wrappers alias its types rather than
        -- wrapping them; a C or C# plugin never resolves it.
        includes(path.join(SDK_DIR, "feather_cpp", "packages", "directxmath.lua"))
        add_requires("directxmath_feather", {system = false, alias = "directxmath"})
    end
    -- host = true: these are build tools this machine runs, not libraries the
    -- plugin links, so a cross-compiling plugin build still gets runnable ones.
    add_requires("mrbind_generators", {system = false, host = true,
        configs = {gen_cpp_rev = feather_gen_cpp_rev(path.join(SDK_DIR, "feather_cpp", "gen_cpp"))}})

    includes(path.join(SDK_DIR, "feather_plugin_rules.lua"))
end
