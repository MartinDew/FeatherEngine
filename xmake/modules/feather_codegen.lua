-- Reflection codegen for one module (tools/codegen/generate_reflection.py).
-- Must be an import()-able module: rule scripts can't see xmake.lua globals.

-- Runs from the module's on_config, before module dependency scanning reads the generated files.
-- A module below reflection sets feather.external_bind; reflection lists it in feather.bind_dirs.
function run(target)
    local root = os.projectdir()
    local argv = {
        path.join(root, "tools", "codegen", "generate_reflection.py"),
        "--project-root", root,
        "--module-path", target:name() .. "=" .. target:scriptdir(),
    }
    if target:values("feather.external_bind") then
        table.insert(argv, "--external-bind")
    end
    for _, dir in ipairs(table.wrap(target:values("feather.bind_dirs"))) do
        table.insert(argv, "--bind-dir")
        table.insert(argv, path.join(root, dir))
    end

    cprint("${cyan}[codegen]${reset} %s", target:name())
    os.vrunv("python3", argv, {curdir = root})
end

function run_embedded_resources()
    local root = os.projectdir()
    cprint("${cyan}[codegen]${reset} generate_embedded_resources.py")
    os.vrunv("python3", {path.join(root, "tools", "codegen", "generate_embedded_resources.py")}, {curdir = root})
end
