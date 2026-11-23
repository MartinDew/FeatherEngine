-- Cross toolchain for Windows targets from Linux/macOS: clang-cl + lld-link over an xwin sysroot.
-- The sysroot is <xwin_dir>/sysroot; xwin_dir defaults to ~/xwin, where tools/cross/install_xwin.py installs.

option("xwin_dir")
    set_default(path.join(os.getenv("HOME") or os.getenv("USERPROFILE") or "", "xwin"))
    set_showmenu(true)
    set_description("xwin install directory used by the xwin toolchain (install_xwin.py --root)")
option_end()

toolchain("xwin")
    set_kind("standalone")
    set_homepage("https://github.com/Jake-Shadle/xwin")
    set_description("clang-cl and lld-link against an xwin Windows SDK/CRT sysroot")

    set_toolset("cc", "clang-cl")
    set_toolset("cxx", "clang-cl")
    set_toolset("ld", "lld-link")
    set_toolset("sh", "lld-link")
    set_toolset("ar", "llvm-ar")
    set_toolset("mrc", "llvm-rc")

    on_check(function (toolchain)
        import("lib.detect.find_program")
        import("core.tool.toolchain", {alias = "toolchain_api"})

        local xwin_dir = path.absolute(get_config("xwin_dir"))
        local sysroot = path.join(xwin_dir, "sysroot")
        if not os.isdir(path.join(sysroot, "VC", "Tools", "MSVC")) then
            raise("xwin sysroot not found at %s, please install xwin using: python %s --root %s",
                sysroot, path.join(os.projectdir(), "tools", "cross", "install_xwin.py"), xwin_dir)
        end
        toolchain:config_set("sysroot", sysroot)

        if not toolchain:is_plat("windows") then
            raise("xwin toolchain only targets windows (got -p %s), use: xmake f -p windows -a x64 --toolchain=xwin", toolchain:plat())
        end
        -- Without -a, xmake falls back to the host arch (x86_64), which is not an MSVC arch name.
        if not table.contains({"x64", "x86", "arm64"}, toolchain:arch()) then
            raise("xwin toolchain needs -a x64, x86 or arm64 (got %s), use: xmake f -p windows -a x64 --toolchain=xwin", toolchain:arch())
        end
        local wrapperdir = path.join(sysroot, "bin", toolchain:arch())
        if not os.isfile(path.join(wrapperdir, "clang-cl")) then
            raise("msvc wrappers not found in %s, please re-run: python %s --root %s",
                wrapperdir, path.join(os.projectdir(), "tools", "cross", "install_xwin.py"), xwin_dir)
        end
        toolchain:config_set("wrapperdir", wrapperdir)

        -- Package builds (cmake, meson) load the msvc toolchain for their env; the sysroot's wrapper layout satisfies it.
        local msvc = toolchain_api.load("msvc", {plat = toolchain:plat(), arch = toolchain:arch()})
        msvc:config_set("sdkdir", sysroot)
        msvc:config_set("__checked", nil)
        if not msvc:check() then
            raise("msvc toolchain check failed for the xwin sysroot %s", sysroot)
        end
        -- CMake reads LIB split on ':' here, so it gets a single dir; compiles and links use /winsysroot instead.
        local flatlib = path.join(sysroot, "lib", toolchain:arch())
        if not os.isdir(flatlib) then
            raise("%s not found, please re-run: python %s --root %s",
                flatlib, path.join(os.projectdir(), "tools", "cross", "install_xwin.py"), xwin_dir)
        end
        local vcvars = msvc:config("vcvars")
        vcvars.LIB = flatlib
        msvc:config_set("vcvars", vcvars)

        -- --bin wins; otherwise the tools come from the LLVM install that llvm-config reports.
        local bindir = get_config("bin")
        if not bindir then
            local llvm_config = find_program("llvm-config")
            bindir = llvm_config and os.iorunv(llvm_config, {"--bindir"}):trim()
        end
        if not bindir or not os.isdir(bindir) then
            raise("LLVM bin directory not found, install LLVM or pass it with --bin=<dir>")
        end
        toolchain:config_set("bindir", bindir)

        -- llvm-rc rejects --version (find_program's default probe), so it is probed with -h.
        local checks = {["clang-cl"] = "--version", ["lld-link"] = "--version", ["llvm-ar"] = "--version", ["llvm-rc"] = "-h"}
        for name, check in pairs(checks) do
            if not find_program(name, {paths = bindir, check = check}) then
                raise("%s not found in %s", name, bindir)
            end
        end
        return true
    end)

    on_load(function (toolchain)
        local sysroot = toolchain:config("sysroot")
        local arch = toolchain:arch()
        local target = ({x64 = "x86_64", x86 = "i686", arm64 = "aarch64"})[arch] or "x86_64"

        -- xmake's response files put one argument per line, which clang-cl splits on; the wrapper joins them.
        local clang_cl = path.join(toolchain:config("wrapperdir"), "clang-cl")
        toolchain:set("toolset", "cc", clang_cl)
        toolchain:set("toolset", "cxx", clang_cl)
        -- The llvm-ar wrapper hands CMake's lib.exe style calls to llvm-lib and xmake's own to llvm-ar.
        toolchain:set("toolset", "ar", path.join(toolchain:config("wrapperdir"), "llvm-ar"))
        toolchain:set("toolset", "mrc", path.join(toolchain:config("wrapperdir"), "llvm-rc"))
        local masm = path.join(toolchain:config("wrapperdir"), arch == "x64" and "ml64.exe" or "ml.exe")
        if os.isfile(masm) then
            toolchain:set("toolset", "as", masm)
        end

        -- /winsysroot resolves the CRT and SDK include/lib dirs itself, so paths with spaces ("Windows Kits") are safe.
        -- Its value stays joined so it survives any response file that is not flattened.
        local compflags = {"--target=" .. target .. "-pc-windows-msvc", "/winsysroot" .. sysroot}
        local linkflags = {"/winsysroot:" .. sysroot, "/machine:" .. arch}
        toolchain:add("cxflags", compflags, {force = true})
        toolchain:add("ldflags", linkflags, {force = true})
        toolchain:add("shflags", linkflags, {force = true})
        -- llvm-ar picks the archive format from the COFF objects, so it needs no machine flag.

        toolchain:set("runtimes", "MD", "MT")
    end)

-- xmake finds the MS STL std.ixx through a target's msvc toolchain (or one named clang-cl), so xwin targets
-- also carry msvc; xwin comes first and supplies every tool, msvc only the std modules and VS env.
rule("xwin.std_modules")
    on_load(function (target)
        if get_config("toolchain") == "xwin" then
            target:set("toolchains", "xwin", "msvc")
        end
    end)
rule_end()
