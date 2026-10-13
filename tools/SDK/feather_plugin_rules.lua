-- Build rules for Feather plugin targets, one per binding language. A plugin
-- declares an ordinary target and adds the rule for the language it is written
-- in; the rule wires in binding generation, the include paths, the platform
-- link setup and -- for C# -- the NativeAOT publish.
--
--     target("my_plugin")
--         add_rules("feather.plugin.c")     -- or .cpp / .cs
--         add_files("src/*.c")
--         -- set_values("feather.api_json", "api/feather_api.json")  -- the default
--
-- feather.plugin.common carries what every language needs; the three language
-- rules add only what is theirs. None of them reaches a FeatherEngine checkout:
-- the designated feather_api.json is the whole interface. See
-- modules/feather_plugin_bindings.lua and FeatherPluginSDK.lua.

-- This file's own directory, captured while it is included. os.scriptdir()
-- inside a rule script resolves to the consumer's project instead.
local SDK_DIR = os.scriptdir()
local SIMPLEMATH_DIR = path.join(SDK_DIR, "feather_cpp", "thirdparty", "SimpleMath")

-- Where the SDK's generators write, under the consumer's build/ (disposable,
-- and out of the source tree the engine's project walk sees). Matches
-- bindings_dir() in modules/feather_plugin_bindings.lua.
local function bindings_dir()
    return path.join(os.projectdir(), "build", "feather_bindings")
end

-- The per-target set_values() a plugin can pass, gathered for feather_plugin_bindings.
--   feather.api_json       the designated API file (default "api/feather_api.json")
--   feather.engine_binary  Windows only: the engine .exe the import table names
--   feather.cs.runtime     C# only: .NET RID to publish for (default: the host's)
--   feather.cs.output_name C# only: filename staged into bin/ (default: lib<name>.so etc.)
local function opts_of(target)
    local function first(name, default)
        local v = target:values(name)
        if type(v) == "table" then
            v = v[1]
        end
        return v or default
    end
    return {
        api_json      = first("feather.api_json", "api/feather_api.json"),
        engine_binary = first("feather.engine_binary"),
        runtime       = first("feather.cs.runtime"),
        output_name   = first("feather.cs.output_name"),
    }
end

-- Everything a plugin needs regardless of language: it is a shared library the
-- engine dlopens, staged flat into the project's bin/, linking nothing of the
-- engine's.
rule("feather.plugin.common")
    on_load(function (target)
        if target:kind() ~= "shared" then
            target:set("kind", "shared")
        end
        target:set("basename", target:name())
        -- mingw prefixes "lib" and MSVC does not -- the .fext manifest names one
        -- file, so pin the spelling that does not vary by toolchain.
        if target:is_plat("windows", "mingw") then
            target:set("prefixname", "")
        end
        -- Flat bin/, not bin/$(mode): the engine finds extensions by walking the
        -- project directory, and a per-mode subdir would leave stale copies of
        -- other configurations there for it to load too.
        target:set("targetdir", path.join(os.projectdir(), "bin"))

        target:add("packages", "mrbind_generators")

        -- Mach-O rejects undefined symbols in a dylib by default; the feather_*
        -- imports are meant to stay undefined and bind to the engine process.
        if target:is_plat("macosx") then
            target:add("shflags", "-undefined", "dynamic_lookup", {force = true})
        end
    end)
rule_end()

-- A C extension: compiles against the generated C headers and resolves the flat
-- feather_* symbols against the engine process that loads it.
rule("feather.plugin.c")
    add_deps("feather.plugin.common")
    on_load(function (target)
        target:add("includedirs", path.join(bindings_dir(), "include"))
    end)
    on_config(function (target)
        import("feather_plugin_bindings")
        local opts = opts_of(target)
        local out = feather_plugin_bindings.generate(target, opts, {})
        feather_plugin_bindings.apply_windows_link(target, opts, out)
    end)
rule_end()

-- A C++ extension: compiles the generated wrappers plus the same vendored
-- SimpleMath the engine did, and resolves the same flat feather_* symbols a C
-- plugin uses -- it shares no C++ ABI with the engine.
rule("feather.plugin.cpp")
    add_deps("feather.plugin.common")
    on_load(function (target)
        assert(os.isdir(path.join(SDK_DIR, "feather_cpp", "gen_cpp")),
            "feather.plugin.cpp: the SDK's feather_cpp/ directory was not vendored")
        target:set("languages", "cxx23")
        -- The single copy of SimpleMath both sides compile: the layouts agree
        -- because the sources do, and the generated headers assert it.
        target:add("files", path.join(SIMPLEMATH_DIR, "SimpleMath.cpp"))
        target:add("includedirs", SIMPLEMATH_DIR,
            path.join(bindings_dir(), "include"), path.join(bindings_dir(), "cpp"))
        target:add("defines", "WIN32_LEAN_AND_MEAN", "NOMINMAX")
        target:add("packages", "directxmath")
        -- Only the entry point is meant to be findable; this plugin's own copy
        -- of SimpleMath's statics stays private to the library.
        if not target:is_plat("windows") then
            target:add("cxflags", "-fvisibility=hidden")
        end
    end)
    on_config(function (target)
        import("feather_plugin_bindings")
        local opts = opts_of(target)
        local out = feather_plugin_bindings.generate(target, opts, {cpp = true})
        feather_plugin_bindings.apply_windows_link(target, opts, out)
    end)
rule_end()

-- A C# extension: xmake's built-in csharp rule turns set_values("csharp.*", ...)
-- into the .csproj; this rule then publishes it with NativeAOT so the result is
-- an ordinary native shared library the engine loads like a C one. The
-- target-level build script installed below suppresses every rule's build
-- script, so the stock csharp rule's `dotnet build` never runs -- only the
-- publish does.
rule("feather.plugin.cs")
    add_deps("feather.plugin.common", "csharp")
    on_load(function (target)
        assert(os.isfile(path.join(SDK_DIR, "feather_cs", "FeatherPluginBootstrap.cs")),
            "feather.plugin.cs: the SDK's feather_cs/ directory was not vendored")

        import("feather_plugin_bindings")

        -- csproj knobs, set only where the consumer left them unset.
        local function default_value(key, value)
            if #table.wrap(target:values(key)) == 0 then
                target:set("values", key, value)
            end
        end
        default_value("csharp.nullable", "disable")
        default_value("csharp.implicit_usings", "enable")
        default_value("csharp.allow_unsafe_blocks", "true")
        default_value("csharp.publish_aot", "true")
        -- NativeAOT is per-RID; the publish below passes -r too, but the csproj
        -- needs it for the restore graph.
        default_value("csharp.runtime_identifier", feather_plugin_bindings.host_dotnet_rid())
        -- Generated bindings warn a lot and are not ours to tidy.
        default_value("csharp.properties", "NoWarn=$(NoWarn);CS0108;CS0114;CS0465;CS8500;CA1416")

        target:set("build", function (target)
            import("feather_plugin_bindings")
            local opts = opts_of(target)
            local out = feather_plugin_bindings.generate(target, opts, {csharp = true})
            feather_plugin_bindings.publish_csharp(target, opts, out)
        end)
    end)
rule_end()
