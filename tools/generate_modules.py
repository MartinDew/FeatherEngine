#!/usr/bin/env python3
"""Auto-generate modules/modules.gen.cpp from discovered feather_module() declarations."""

import argparse
import re
import sys
from pathlib import Path

GENERATED_NOTICE = (
    "// THIS FILE IS AUTO-GENERATED — DO NOT EDIT BY HAND.\n"
    "// Re-run generate_modules.py to refresh.\n"
)


def find_modules(modules_path: Path) -> list[str]:
    modules = []
    for cmake_file in sorted(modules_path.glob("*/CMakeLists.txt")):
        text = cmake_file.read_text(encoding="utf-8")
        match = re.search(r"feather_module\s*\(\s*(\w+)", text)
        if match:
            modules.append(match.group(1))
    return modules


def generate_cpp(modules: list[str]) -> str:
    lines = [
        GENERATED_NOTICE,
        '#include "modules.gen.h"',
        '#include <framework/assert.h>',
        "",
    ]
    for name in modules:
        lines.append(f"#if {name}_ENABLED")
        lines.append(f"#include <modules/{name}/register_module.h>")
        lines.append("#endif")
    lines += [
        "",
        "namespace feather {",
        "",
        "static bool registered = false;",
        "",
        "void register_modules() {",
        '\tfassert(!registered, "modules already registered but register_modules was called!");',
    ]
    for name in modules:
        lines.append(f"#if {name}_ENABLED")
        lines.append(f"\tregister_{name}();")
        lines.append("#endif")
    lines += [
        "",
        "\tregistered = true;",
        "}",
        "",
        "void unregister_modules() {",
        '\tfassert(registered, "modules not registered but unregister was called!");',
    ]
    for name in modules:
        lines.append(f"#if {name}_ENABLED")
        lines.append(f"\tunregister_{name}();")
        lines.append("#endif")
    lines += [
        "\tregistered = false;",
        "}",
        "",
        "} //namespace feather",
        "",
    ]
    return "\n".join(lines)


def write_if_changed(path: Path, content: str) -> bool:
    if path.exists() and path.read_text(encoding="utf-8") == content:
        return False
    path.write_text(content, encoding="utf-8")
    return True


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--modules-path", type=Path, default=Path("./modules"))
    args = parser.parse_args()

    modules_path = args.modules_path.resolve()
    if not modules_path.is_dir():
        print(f"[ERROR] modules path not found: {modules_path}", file=sys.stderr)
        sys.exit(1)

    modules = find_modules(modules_path)
    print(f"  [modules] found: {modules if modules else '(none)'}")

    cpp = generate_cpp(modules)
    out_cpp = modules_path / "modules.gen.cpp"
    changed = write_if_changed(out_cpp, cpp)
    print(f"  [modules] modules.gen.cpp {'updated' if changed else 'up to date.'}")

    print("Done.")


if __name__ == "__main__":
    main()
