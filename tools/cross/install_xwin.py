#!/usr/bin/env python3
"""Install xwin and splat the MSVC CRT + Windows SDK into ~/xwin for the `xwin` xmake toolchain.

Layout: ~/xwin/bin/xwin (the tool), ~/xwin/cache (downloads), ~/xwin/sysroot (what the toolchain uses).
The sysroot also gets msvc-wine style wrappers in sysroot/bin/<arch> (cl.exe, link.exe, lib.exe, ...)
over the LLVM tools, which is what xmake's msvc toolchain looks for when building packages on Linux.
"""

import argparse
import fnmatch
import shutil
import subprocess
import sys
import zipfile
from pathlib import Path

DEFAULT_ROOT = Path.home() / "xwin"

# xwin --arch name -> (MS arch dir name, clang target triple)
ARCHS = {
    "x86_64": ("x64", "x86_64-pc-windows-msvc"),
    "x86": ("x86", "i686-pc-windows-msvc"),
    "aarch64": ("arm64", "aarch64-pc-windows-msvc"),
    "aarch": ("arm", "armv7-pc-windows-msvc"),
}

# llvm-ar receives xmake's "cr" style and CMake's lib.exe style (/nologo /out:...), and llvm-lib only takes the latter.
LLVM_AR_WRAPPER = """#!/bin/sh
case "$1" in
    [/-][nN][oO][lL][oO][gG][oO]|[/-][oO][uU][tT]:*|[/-][mM][aA][cC][hH][iI][nN][eE]:*) exec "{bin}/llvm-lib" "$@" ;;
esac
exec "{bin}/llvm-ar" "$@"
"""


# clang-cl ends an argument at each line end of a response file, and xmake writes them one argument per line.
CLANG_CL_WRAPPER = """#!/bin/sh
tmps=""
for arg; do
    shift
    case "$arg" in
        @*) tmp=$(mktemp); tr '\\n' ' ' < "${{arg#@}}" > "$tmp"; tmps="$tmps $tmp"; set -- "$@" "@$tmp" ;;
        *) set -- "$@" "$arg" ;;
    esac
done
"{bin}/clang-cl" "$@"
status=$?
[ -n "$tmps" ] && rm -f $tmps
exit $status
"""


def run(cmd: list[str]) -> None:
    print("+", " ".join(cmd), flush=True)
    subprocess.run(cmd, check=True)


def find_llvm_bindir(explicit: Path | None) -> Path:
    if explicit:
        return explicit.expanduser().resolve()
    llvm_config = shutil.which("llvm-config")
    if llvm_config:
        return Path(subprocess.run([llvm_config, "--bindir"], check=True, capture_output=True, text=True).stdout.strip())
    clang_cl = shutil.which("clang-cl")
    if clang_cl:
        return Path(clang_cl).resolve().parent
    sys.exit("error: LLVM not found; install it or pass --llvm-bin")


def write_script(file: Path, text: str) -> None:
    file.write_text(text)
    file.chmod(0o755)


def write_wrappers(sysroot: Path, xwin_arch: str, llvm_bin: Path) -> None:
    ms_arch, triple = ARCHS[xwin_arch]
    bindir = sysroot / "bin" / ms_arch
    bindir.mkdir(parents=True, exist_ok=True)

    tools = {
        "cl": f'"{llvm_bin}/clang-cl" --target={triple}',
        "link": f'"{llvm_bin}/lld-link"',
        "lib": f'"{llvm_bin}/llvm-lib"',
        # assimp's .rc files are not ASCII, so the code page has to be explicit.
        "rc": f'"{llvm_bin}/llvm-rc" /C 1252',
        "mt": f'"{llvm_bin}/llvm-mt"',
        "ml64" if ms_arch == "x64" else "ml": f'"{llvm_bin}/llvm-ml64"' if ms_arch == "x64" else f'"{llvm_bin}/llvm-ml"',
    }
    for name, command in tools.items():
        for filename in (name, name + ".exe"):
            write_script(bindir / filename, f'#!/bin/sh\nexec {command} "$@"\n')
    write_script(bindir / "llvm-ar", LLVM_AR_WRAPPER.format(bin=llvm_bin))
    write_script(bindir / "llvm-rc", f'#!/bin/sh\nexec {tools["rc"]} "$@"\n')
    write_script(bindir / "clang-cl", CLANG_CL_WRAPPER.format(bin=llvm_bin))
    print(f"wrote msvc wrappers to {bindir}")


def add_lib_case_variants(sysroot: Path) -> None:
    # lld-link is case sensitive on Linux; sources spell import libs every way (Dbghelp.lib, D3D12.LIB, ...).
    count = 0
    for lib in [p for p in sysroot.rglob("*") if p.suffix.lower() == ".lib" and p.is_file()]:
        stem = lib.stem.lower()
        for name in {stem + ".lib", stem.capitalize() + ".lib", stem.upper() + ".lib", stem.upper() + ".LIB"}:
            alias = lib.with_name(name)
            if not alias.exists() and not alias.is_symlink():
                alias.symlink_to(lib.name)
                count += 1
    print(f"added {count} import library case variants")


def write_flat_libdir(sysroot: Path, xwin_arch: str) -> None:
    # One directory holding every import lib, because CMake on Linux splits the ';'-joined msvc-wine LIB on ':'.
    ms_arch = ARCHS[xwin_arch][0]
    sources = sorted(sysroot.glob(f"VC/Tools/MSVC/*/lib/{ms_arch}")) + sorted(sysroot.glob(f"Windows Kits/10/Lib/*/*/{ms_arch}"))
    flat = sysroot / "lib" / ms_arch
    if flat.exists():
        shutil.rmtree(flat)
    flat.mkdir(parents=True)
    for source in sources:
        for lib in source.iterdir():
            alias = flat / lib.name
            if lib.suffix.lower() == ".lib" and not alias.is_symlink():
                alias.symlink_to(lib.resolve())
    print(f"linked import libraries into {flat}")


def extract_std_modules(root: Path, sysroot: Path) -> None:
    # splat only keeps the CRT's include/ and lib/, so std.ixx is copied from the cached CRT.Headers package.
    vsixes = sorted((root / "cache" / "dl").glob("Microsoft.VC.*.CRT.Headers.base.vsix"))
    toolsets = sorted((sysroot / "VC" / "Tools" / "MSVC").iterdir())
    if not vsixes or len(toolsets) != 1:
        sys.exit("error: cannot locate the CRT.Headers package or a single MSVC toolset; re-run with --force")
    modulesdir = toolsets[0] / "modules"
    modulesdir.mkdir(exist_ok=True)
    with zipfile.ZipFile(vsixes[-1]) as vsix:
        members = [m for m in vsix.namelist() if fnmatch.fnmatch(m, "Contents/VC/Tools/MSVC/*/modules/*") and not m.endswith("/")]
        for member in members:
            (modulesdir / Path(member).name).write_bytes(vsix.read(member))
    if not (modulesdir / "std.ixx").is_file():
        sys.exit(f"error: std.ixx not found in {vsixes[-1].name}")
    print(f"extracted {len(members)} std module files to {modulesdir}")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--root", type=Path, default=DEFAULT_ROOT, help=f"install directory (default: {DEFAULT_ROOT})")
    parser.add_argument("--arch", default="x86_64", help="xwin --arch value, comma separated (default: x86_64)")
    parser.add_argument("--llvm-bin", type=Path, help="LLVM bin directory for the wrappers (default: llvm-config --bindir)")
    parser.add_argument("--force", action="store_true", help="re-splat even if the sysroot already exists")
    args = parser.parse_args()

    archs = [a.strip() for a in args.arch.split(",") if a.strip()]
    for arch in archs:
        if arch not in ARCHS:
            sys.exit(f"error: unknown arch {arch!r}, expected one of {', '.join(ARCHS)}")

    root: Path = args.root.expanduser().resolve()
    xwin = root / "bin" / "xwin"
    sysroot = root / "sysroot"
    llvm_bin = find_llvm_bindir(args.llvm_bin)

    if not xwin.exists():
        cargo = shutil.which("cargo")
        if not cargo:
            sys.exit("error: cargo not found; install Rust (https://rustup.rs) and re-run")
        run([cargo, "install", "--locked", "xwin", "--root", str(root)])

    if sysroot.exists() and not args.force:
        print(f"sysroot already present at {sysroot} (pass --force to rebuild)")
    else:
        # splat asks for the Microsoft license on the terminal, so this must run interactively.
        run([
            str(xwin), "--cache-dir", str(root / "cache"), "--arch", ",".join(archs),
            "splat", "--output", str(sysroot),
            "--use-winsysroot-style", "--preserve-ms-arch-notation", "--include-debug-libs",
        ])

    extract_std_modules(root, sysroot)
    add_lib_case_variants(sysroot)
    for arch in archs:
        write_flat_libdir(sysroot, arch)
        write_wrappers(sysroot, arch, llvm_bin)
    print(f"done: sysroot at {sysroot}")


if __name__ == "__main__":
    try:
        main()
    except subprocess.CalledProcessError as e:
        sys.exit(f"error: command failed with exit code {e.returncode}")
