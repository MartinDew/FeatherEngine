-- Template for a Feather extension written in C#.
--
-- Copy this directory into your project, then:
--   1. rename the target (marked TODO below); write your own
--      [FeatherComponent]/[FeatherSystem]/[FeatherInit] types in
--      src/MyPlugin.cs -- no entry point or manifest name to invent, the SDK's
--      bootstrap supplies those (see that file's own comment).
--   2. copy the engine's published API description into api/:
--        cd <FeatherEngine> && xmake export-api
--        cp build/bindings/dist/feather_api.* <your project>/api/
--   3. copy the SDK itself in, once:
--        cp -r <FeatherEngine>/tools/SDK/{FeatherPluginSDK.lua,feather_plugin_rules.lua,modules,packages,feather_cs} <your project>/sdk/
--   4. xmake     (requires the .NET SDK on PATH)
--
-- xmake's built-in csharp rule turns the set_values("csharp.*", ...) below into
-- the .csproj; feather.plugin.cs then publishes it with NativeAOT, so the result
-- is an ordinary native shared library: the engine loads it exactly like a C
-- extension and hosts no .NET runtime of its own.
set_xmakever("3.1.0")
set_project("my_plugin") -- TODO: rename
add_rules("mode.debug", "mode.releasedbg", "mode.release")

includes("sdk/FeatherPluginSDK.lua")
feather_plugin_sdk_init()

target("my_plugin") -- TODO: rename
    add_rules("feather.plugin.cs")
    add_files("src/*.cs")
    -- feather.plugin.cs seeds PublishAot, AllowUnsafeBlocks, Nullable and the
    -- host RuntimeIdentifier; add or override any csproj property here, e.g.
    --   set_values("csharp.target_framework", "net10.0")
    --   set_values("feather.api_json", "api/feather_api.json")  -- the default
