#!/usr/bin/env python3
"""
generate_register_types.py

Scans each top-level subfolder under `core/` for FCLASS / FABSTRACT / FSINGLETON
macro usages in header files, then emits:

  core/<subfolder>/register_<subfolder>_types.gen.h
  core/<subfolder>/register_<subfolder>_types.gen.cpp

Usage:
    python generate_register_types.py [--core-path PATH]

  --core-path  Path to the `core` directory (default: ./core)
"""

import argparse
import re
import sys
from dataclasses import dataclass, field
from pathlib import Path

# ---------------------------------------------------------------------------
# Configuration
# ---------------------------------------------------------------------------

TOP_SUBFOLDERS = ["framework", "main", "math", "rendering", "resources"]

# Supported macros and the ClassDB method they map to
MACRO_TO_METHOD = {
    "FCLASS":     "register_class",
    "FABSTRACT":  "register_abstract_class",
    "FSINGLETON": "register_singleton_class",
}

# Regex that matches any of the macros above.
# Captures: (macro_name, class_name, parent_name)
# Handles optional whitespace / line continuations.
_MACRO_RE = re.compile(
    r'\b(FCLASS|FABSTRACT|FSINGLETON)\s*\(\s*(\w+)\s*,\s*(\w+)\s*\)',
    re.MULTILINE,
)

GENERATED_NOTICE = """\
// THIS FILE IS AUTO-GENERATED — DO NOT EDIT BY HAND.
// Re-run generate_register_types.py to refresh.
"""

# ---------------------------------------------------------------------------
# Data model
# ---------------------------------------------------------------------------

@dataclass
class ClassEntry:
    class_name: str
    parent_name: str
    method: str          # register_class / register_abstract_class / register_singleton_class
    source_header: Path  # relative path used for #include


# ---------------------------------------------------------------------------
# Parsing
# ---------------------------------------------------------------------------

def _strip_define_lines(text: str) -> str:
    """Remove any line that contains a #define involving FCLASS."""
    return "\n".join(
        line for line in text.splitlines()
        if not re.search(r'#\s*define\s+\w*FCLASS', line)
    )

def scan_folder(folder: Path) -> list[ClassEntry]:
    """Recursively scan *folder* for header files and return all ClassEntry items found."""
    entries: list[ClassEntry] = []

    for header in sorted(folder.rglob("*.h")) + sorted(folder.rglob("*.hpp")):
        try:
            text = header.read_text(encoding="utf-8", errors="replace")
        except OSError as exc:
            print(f"  [WARN] Cannot read {header}: {exc}", file=sys.stderr)
            continue

        text = _strip_define_lines(text)

        for match in _MACRO_RE.finditer(text):
            macro, cls, parent = match.group(1), match.group(2), match.group(3)
            if cls == "_name":
                continue
            entries.append(ClassEntry(
                class_name=cls,
                parent_name=parent,
                method=MACRO_TO_METHOD[macro],
                source_header=header,
            ))

    return entries


# ---------------------------------------------------------------------------
# Topological sort (parent before child)
# ---------------------------------------------------------------------------

def topological_sort(entries: list[ClassEntry]) -> list[ClassEntry]:
    """
    Sort entries so that a parent class is always registered before its children.
    Classes whose parent is not in the local list are treated as already registered
    externally and are placed first (stable relative order preserved).
    """
    known = {e.class_name for e in entries}
    result: list[ClassEntry] = []
    visited: set[str] = set()

    def visit(entry: ClassEntry):
        if entry.class_name in visited:
            return
        # If the parent is known locally, ensure it's visited first
        if entry.parent_name in known:
            parent_entry = next(e for e in entries if e.class_name == entry.parent_name)
            visit(parent_entry)
        visited.add(entry.class_name)
        result.append(entry)

    for e in entries:
        visit(e)

    return result


# ---------------------------------------------------------------------------
# Code generation
# ---------------------------------------------------------------------------

def make_relative_include(header: Path, core_path: Path) -> str:
    """Return a path string suitable for use in an #include directive."""
    try:
        return header.relative_to(core_path).as_posix()
    except ValueError:
        return header.as_posix()


def generate_header(subfolder_name: str) -> str:
    func_name = f"register_{subfolder_name}_types"
    guard = f"REGISTER_{subfolder_name.upper()}_TYPES_GEN_H"
    return (
        f"{GENERATED_NOTICE}"
        f"#pragma once\n"
        f"\n"
        f"namespace feather {{\n"
        f"\n"
        f"extern void {func_name}();\n"
        f"\n"
        f"}} // namespace feather\n"
        f"\n"
    )


def generate_cpp(subfolder_name: str, entries: list[ClassEntry], core_path: Path) -> str:
    func_name = f"register_{subfolder_name}_types"
    gen_header = f"register_{subfolder_name}_types.gen.h"

    # Collect unique headers to include, in the order they first appear
    seen_headers: set[Path] = set()
    ordered_headers: list[str] = []
    for e in entries:
        if e.source_header not in seen_headers:
            seen_headers.add(e.source_header)
            ordered_headers.append(make_relative_include(e.source_header, core_path))

    lines: list[str] = [GENERATED_NOTICE]

    lines.append(f'#include "{gen_header}"')
    lines.append('#include <core/main/class_db.h>')
    lines.append("")

    for inc in ordered_headers:
        lines.append(f'#include "{inc}"')

    lines.append("")
    lines.append("namespace feather {")
    lines.append("")
    lines.append(f"void {func_name}() {{")

    if entries:
        for e in entries:
            lines.append(f"\tClassDB::{e.method}<{e.class_name}>();")
    else:
        lines.append("\t// No reflected classes found in this module.")

    lines.append("}")
    lines.append("")
    lines.append("} // namespace feather")
    lines.append("")

    return "\n".join(lines)


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(description="Generate ClassDB registration files.")
    parser.add_argument(
        "--core-path",
        type=Path,
        default=Path("./core"),
        help="Path to the core directory (default: ./core)",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Print what would be written without touching the filesystem.",
    )
    args = parser.parse_args()

    core_path: Path = args.core_path.resolve()

    if not core_path.is_dir():
        print(f"[ERROR] core path not found: {core_path}", file=sys.stderr)
        sys.exit(1)

    print(f"Scanning core path: {core_path}\n")

    for subfolder_name in TOP_SUBFOLDERS:
        subfolder = core_path / subfolder_name
        if not subfolder.is_dir():
            print(f"  [SKIP] {subfolder_name}/ — directory not found.")
            continue

        print(f"  [{subfolder_name}] scanning...", end=" ")
        raw_entries = scan_folder(subfolder)
        sorted_entries = topological_sort(raw_entries)
        print(f"found {len(sorted_entries)} class(es).")

        for e in sorted_entries:
            rel = make_relative_include(e.source_header, core_path)
            print(f"    -> {e.method}<{e.class_name}>  (parent: {e.parent_name})  [{rel}]")

        header_text = generate_header(subfolder_name)
        cpp_text    = generate_cpp(subfolder_name, sorted_entries, core_path)

        out_h   = subfolder / f"register_{subfolder_name}_types.gen.h"
        out_cpp = subfolder / f"register_{subfolder_name}_types.gen.cpp"

        if args.dry_run:
            print(f"\n    --- {out_h} ---")
            print(header_text)
            print(f"    --- {out_cpp} ---")
            print(cpp_text)
        else:
            out_h.write_text(header_text, encoding="utf-8")
            out_cpp.write_text(cpp_text, encoding="utf-8")
            print(f"    [x]  wrote {out_h.name}")
            print(f"    [x]  wrote {out_cpp.name}")

        print()

    print("Done.")


if __name__ == "__main__":
    main()