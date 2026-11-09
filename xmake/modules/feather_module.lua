-- Module bookkeeping shared by the feather.* rules. A module is a target named
-- after its directory, with an enable_<name> option in the "feather_modules"
-- category whose add_deps() lists the options of the modules it depends on.

import("core.project.config")
import("core.project.project")

local CATEGORY = "feather_modules"

function macro_prefix(name)
    return "FEATHER_" .. name:upper()
end

local function _module_options()
    local options = {}
    for _, opt in pairs(project.options()) do
        if opt:get("category") == CATEGORY then
            table.insert(options, opt)
        end
    end
    return options
end

-- A command-line --enable_x=n stays readonly in the config cache, so forcing the
-- option on does not survive into the build; the effective state is computed here.
function enabled(name)
    local wanted = "enable_" .. name
    for _, opt in ipairs(_module_options()) do
        if has_config(opt:name()) then
            if opt:name() == wanted then
                return true
            end
            for _, dep in ipairs(opt:orderdeps()) do
                if dep:name() == wanted then
                    return true
                end
            end
        end
    end
    return false
end

function enabled_modules()
    local names = {}
    for _, opt in ipairs(_module_options()) do
        local name = opt:name():match("^enable_(.+)$")
        if name and enabled(name) then
            table.insert(names, name)
        end
    end
    table.sort(names)
    return names
end

-- The API macro is defined on the command line: the export form for the module
-- itself, the dependent form (interface define) for everything that depends on it.
local function _api_defines(target, shared)
    if not shared then
        return "", ""
    end
    -- No dllimport for dependents: clang doesn't emit a dllexport class's inline members from a
    -- module interface, so each binary keeps its own; functions resolve through import thunks.
    if target:is_plat("windows", "mingw") then
        return "__declspec(dllexport)", ""
    end
    local visible = "__attribute__((visibility(\"default\")))"
    return visible, visible
end

function load(target)
    local name = target:name()
    local dir = target:scriptdir()
    target:set("enabled", enabled(name))

    local shared = get_config("module_kind") == "shared"
    target:set("kind", shared and "shared" or "static")
    target:set("group", "modules")
    if shared then
        target:set("targetdir", path.join(config.builddir(), "bin"))
        if target:is_plat("linux") then
            target:add("rpathdirs", "$ORIGIN")
        end
        -- Each binary needs its own copy of the allocator overrides; framework already globs it.
        if name ~= "framework" then
            target:add("files", path.join(os.projectdir(), "framework", "alloc_overrides.cpp"))
        end
    end

    local prefix = macro_prefix(name)
    local export_api, import_api = _api_defines(target, shared)
    -- Identical values must be added once: xmake would keep only the interface entry.
    if export_api == import_api then
        target:add("defines", prefix .. "_API=" .. export_api, {public = true})
    else
        target:add("defines", prefix .. "_API=" .. export_api)
        target:add("defines", prefix .. "_API=" .. import_api, {interface = true})
    end

    target:add("files", path.join(dir, "**.cppm|**.gen.cppm"), {public = true})
    target:add("files", path.join(dir, "**.cpp|**.gen.cpp"))
    -- Codegen output only exists after the first configure.
    local register = path.join(dir, "register_" .. name .. "_types.gen")
    target:add("files", register .. ".cppm", {always_added = true, public = true})
    target:add("files", register .. ".cpp", {always_added = true})
end

-- Every target archives its own copy of the std module objects; whole-archive linking
-- would pull in each one, so module archives drop them and the executable keeps its own.
function strip_std_objects(target)
    if not target:is_static() then
        return
    end
    local objectfiles = target:objectfiles()
    for i = #objectfiles, 1, -1 do
        local name = path.filename(objectfiles[i])
        if name:match("^std%.") and (name:match("%.o$") or name:match("%.obj$")) then
            table.remove(objectfiles, i)
        end
    end
end

-- Static module libraries link whole-archive: nothing may be dropped, since the executable
-- exports engine symbols to dlopen'd project libraries (-rdynamic).
function link_whole(target)
    local names, files = {}, {}
    for _, dep in ipairs(target:orderdeps()) do
        if dep:is_static() and dep:get("group") == "modules" then
            table.insert(names, dep:name())
            table.insert(files, dep:targetfile())
        end
    end
    if #names == 0 then
        return
    end
    if target:is_plat("windows") then
        for _, file in ipairs(files) do
            target:add("ldflags", "/WHOLEARCHIVE:" .. file, {force = true})
        end
    elseif target:is_plat("macosx") then
        for _, file in ipairs(files) do
            target:add("ldflags", "-Wl,-force_load," .. file, {force = true})
        end
    elseif target:is_plat("mingw") then
        -- xmake's linkgroups ignore `whole` on mingw.
        target:add("ldflags", "-Wl,--whole-archive", files, "-Wl,--no-whole-archive", {force = true})
    else
        target:add("linkgroups", names, {whole = true})
    end
end
