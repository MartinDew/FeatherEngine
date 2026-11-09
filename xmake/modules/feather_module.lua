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
-- itself, the import form (interface define) for everything that depends on it.
local function _api_defines(target, shared)
    if not shared then
        return "", ""
    end
    if target:is_plat("windows", "mingw") then
        return "__declspec(dllexport)", "__declspec(dllimport)"
    end
    local visible = "__attribute__((visibility(\"default\")))"
    return visible, visible
end

function load(target)
    local name = target:name()
    local dir = target:scriptdir()
    target:set("enabled", enabled(name))

    -- static maps to object: xmake links every object of an object dep into the final
    -- binary, so self-registering translation units are never dropped like from a .a.
    local shared = get_config("module_kind") == "shared"
    target:set("kind", shared and "shared" or "object")
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

