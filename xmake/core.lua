-- feather_core: the feather.core module plus the engine's public build surface
-- (defines, include dirs, packages). Shared with tools/SDK/FeatherSDK.lua.
--
-- os.scriptdir(), not os.projectdir(): the latter would resolve to a
-- CONSUMER's repo when this file is includes()'d cross-repo.
local ROOT = path.directory(os.scriptdir())
local function root(p) return path.join(ROOT, p) end

target("feather_core")
    -- object: a static archive would drop TUs nothing references directly.
    set_kind("object")
    add_files(root("core/**.cppm"), {public = true})
    add_files(root("core/**.cpp"))
    -- Codegen output only exists after the first build, one per core subfolder.
    for _, dir in ipairs(os.dirs(root("core/*"))) do
        add_files(path.join(dir, "register_" .. path.filename(dir) .. "_types.gen.cpp"), {always_added = true})
    end

    -- Public: BMIs are rebuilt in each dependent, and a mismatch is an ODR bug.
    add_defines("EDITOR_BUILD=" .. (has_config("editor_build") and "1" or "0"), {public = true})
    add_defines("FEATHER_BUILDING_ENGINE", {public = true}) -- flips FEATHER_API to dllexport
    add_defines(is_mode("release") and "PRODUCTION" or "BETA", {public = true})
    add_includedirs(ROOT, root("core"), root("thirdparty/SimpleMath"), {public = true})

    add_deps("simplemath", {public = true})
    add_packages("directxmath", "taywee_args", {public = true})
    -- flecs/sdl3 own process-global state: shared libs on windows/mingw, elsewhere headers
    -- only so a DLL binds to the host exe's copy via -rdynamic.
    if is_plat("windows", "mingw") then
        add_packages("flecs", "sdl3", {public = true})
    else
        add_packages("flecs", "sdl3", {public = true, links = {}})
    end
    add_packages("assimp")

    -- import() only exists inside the hook's own closure.
    on_config(function (target)
        import("feather_codegen")
        import("feather_flags")

        local module_dirs = {}
        local vex_dir = path.join(ROOT, "modules", "vex_renderer")
        if not is_plat("macosx") and has_config("enable_vex_renderer") and os.isdir(vex_dir) then
            table.insert(module_dirs, vex_dir)
        end
        feather_codegen.run_core_codegen(module_dirs, {feather_root = ROOT})

        cprint("${cyan}[codegen]${reset} generate_embedded_resources.py")
        os.vrunv("python3", {path.join(ROOT, "tools", "codegen", "generate_embedded_resources.py")}, {curdir = ROOT})

        feather_flags.apply(target)
    end)
target_end()
