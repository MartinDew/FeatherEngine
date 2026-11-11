-- after_check script of every enable_<module> option: reports the dependencies an
-- enabled module turns back on (feather_module.enabled() does the actual enabling).
import("core.project.project")

function main(opt)
    if not opt:enabled() then
        return
    end
    for _, depname in ipairs(table.wrap(opt:get("deps"))) do
        local dep = project.option(depname)
        if dep and not dep:enabled() then
            cprint("${yellow}note:${clear} %s requires %s, enabling it", opt:name(), depname)
        end
    end
end
