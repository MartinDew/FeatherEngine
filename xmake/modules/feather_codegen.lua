-- Reflection codegen for one module (tools/codegen/generate_reflection.py).
-- Must be an import()-able module: rule scripts can't see xmake.lua globals.

import("core.project.depend")

local function _codegen_scripts(root)
    return os.files(path.join(root, "tools", "codegen", "**.py"))
end

-- Files the generator reads in dir: what generate_reflection.py's find_headers() scans.
local function _sources(dir)
    local files = {}
    for _, pattern in ipairs({"**.h", "**.hpp", "**.cppm"}) do
        for _, file in ipairs(os.files(path.join(dir, pattern))) do
            if not file:find(".gen.", 1, true) then
                table.insert(files, file)
            end
        end
    end
    return files
end

-- Reruns the generator only when an input, the generator, the arguments or the input file set
-- changed (depend compares the recorded file list too), or a recorded output went missing.
local function _run_if_changed(target, name, argv, inputs, outputs_dir)
    table.sort(inputs)
    local dependfile = target:dependfile("codegen_" .. name)
    local outputs_missing = false
    for _, output in ipairs(table.wrap((depend.load(dependfile) or {}).outputs)) do
        if not os.isfile(output) then
            outputs_missing = true
            break
        end
    end
    depend.on_changed(function ()
        cprint("${cyan}[codegen]${reset} %s", name)
        os.vrunv("python3", argv, {curdir = os.projectdir()})
        return {outputs = os.files(path.join(outputs_dir, "**.gen.*"))}
    end, {dependfile = dependfile, files = inputs, values = argv, changed = outputs_missing})
end

-- Runs from the module's on_config, before module dependency scanning reads the generated files.
-- A module below reflection sets feather.external_bind; reflection lists it in feather.bind_dirs.
function run(target)
    local root = os.projectdir()
    local argv = {
        path.join(root, "tools", "codegen", "generate_reflection.py"),
        "--project-root", root,
        "--module-path", target:name() .. "=" .. target:scriptdir(),
    }
    local inputs = table.join(_sources(target:scriptdir()), _codegen_scripts(root))
    if target:values("feather.external_bind") then
        table.insert(argv, "--external-bind")
    end
    for _, dir in ipairs(table.wrap(target:values("feather.bind_dirs"))) do
        local bind_dir = path.join(root, dir)
        table.insert(argv, "--bind-dir")
        table.insert(argv, bind_dir)
        table.join2(inputs, _sources(bind_dir))
    end
    _run_if_changed(target, target:name(), argv, inputs, target:scriptdir())
end

-- Embeds every file under raw_resources/ (tools/codegen/generate_embedded_resources.py).
function run_embedded_resources(target)
    local root = os.projectdir()
    local resources_dir = path.join(root, "raw_resources")
    local script = path.join(root, "tools", "codegen", "generate_embedded_resources.py")
    local inputs = {script}
    for _, file in ipairs(os.files(path.join(resources_dir, "**"))) do
        if not file:find(".gen.", 1, true) then
            table.insert(inputs, file)
        end
    end
    _run_if_changed(target, "embedded_resources", {script}, inputs, resources_dir)
end
