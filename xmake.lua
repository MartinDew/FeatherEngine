set_xmakever("2.9.0")
set_project("feather")
set_version("1.0.0")
set_languages("cxx23", "clatest")
-- The engine uses `import std;` / `import std.compat;`.
set_policy("build.c++.modules.std", true)

-- import()-able script modules (xmake/modules/*.lua): feather_module, feather_codegen, feather_flags.
add_moduledirs("xmake/modules")

includes("xmake/options.lua")
includes("xmake/rules.lua")

-- ---- Build modes --------------------------------------------------------
-- debug      -> CMake Debug       (-O0, symbols, BETA, no NDEBUG)
-- releasedbg -> CMake Development (-O2, symbols, BETA, no NDEBUG)
-- release    -> CMake Release     (-O3, NDEBUG)
add_rules("mode.debug", "mode.releasedbg", "mode.release")

-- mode.release hides symbols by default, which defeats -rdynamic and breaks
-- dlopen'd project DLLs. Set globally so linked-in static libs match too.
if is_mode("release") then
    set_symbols("none")
end

-- Keep compile_commands.json up to date for clangd / clang-tidy
add_rules("plugin.compile_commands.autoupdate", {outputdir = "$(builddir)"})

-- ---- Global defines -----------------------------------------------------
-- Every module sees the same values: BMIs are shared across targets, and a mismatch is an ODR bug.
add_defines("EDITOR_BUILD=" .. (has_config("editor_build") and "1" or "0"))
add_defines(is_mode("release") and "PRODUCTION" or "BETA")
if has_config("double_precision") then
    add_defines("FEATHER_DOUBLE_PRECISION")
end
if is_plat("windows") then
    add_defines("NOMINMAX", "WIN32_LEAN_AND_MEAN")
end
if is_plat("macosx") then
    add_cxflags("-fexperimental-library", {force = true})
end
-- Module sources include each other's plain headers as <module/header.h>.
add_includedirs(".")

-- ---- LTO ----------------------------------------------------------------
if has_config("production") or has_config("use_lto") then
    set_policy("build.optimization.lto", true)
end

-- ---- Static C++ runtime on MSVC (set_runtimes must be global) -----------
if has_config("production") or has_config("static_cpp") then
    set_runtimes(is_mode("debug") and "MTd" or "MT")
end

-- ---- Third-party packages -----------------------------------------------
includes("thirdparty/xmake.lua")

-- ---- Modules ------------------------------------------------------------
-- Engine modules, lowest layer first; each declares its own deps in its xmake.lua.
includes("framework", "math", "reflection", "core", "resources", "rendering", "world", "main")

-- ---- Executable (default target) ----------------------------------------
includes("app")

-- Optional modules, linked into the executable when enabled. After the executable,
-- so their xmake.lua can add packages and rules to it.
includes("modules/*/xmake.lua")
