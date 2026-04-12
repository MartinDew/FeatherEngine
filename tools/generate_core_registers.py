#!/usr/bin/env python3
"""
generate_register_types.py - Updated with Timestamp Protection
"""

import argparse
import re
import sys
from dataclasses import dataclass
from pathlib import Path

# ... [Keep TOP_SUBFOLDERS, MACRO_TO_METHOD, _MACRO_RE, and GENERATED_NOTICE as they were] ...
TOP_SUBFOLDERS = ["framework", "main", "math", "rendering", "resources"]
MACRO_TO_METHOD = {
    "FCLASS":     "register_class",
    "FABSTRACT":  "register_abstract_class",
    "FSINGLETON": "register_singleton_class",
}
_MACRO_RE = re.compile(
    r'\b(FCLASS|FABSTRACT|FSINGLETON)\s*\(\s*(\w+)\s*,\s*(\w+)\s*\)',
    re.MULTILINE,
)
GENERATED_NOTICE = "// THIS FILE IS AUTO-GENERATED — DO NOT EDIT BY HAND.\n// Re-run generate_register_types.py to refresh.\n"

@dataclass
class ClassEntry:
    class_name: str
    parent_name: str
    method: str
    source_header: Path

# ... [Keep _strip_define_lines, scan_folder, and topological_sort as they were] ...

def _strip_define_lines(text: str) -> str:
    return "\n".join(line for line in text.splitlines() if not re.search(r'#\s*define\s+\w*FCLASS', line))

def scan_folder(folder: Path) -> list[ClassEntry]:
    entries: list[ClassEntry] = []
    for header in sorted(folder.rglob("*.h")) + sorted(folder.rglob("*.hpp")):
        try:
            text = header.read_text(encoding="utf-8", errors="replace")
        except OSError: continue
        text = _strip_define_lines(text)
        for match in _MACRO_RE.finditer(text):
            macro, cls, parent = match.group(1), match.group(2), match.group(3)
            if cls == "_name": continue
            entries.append(ClassEntry(cls, parent, MACRO_TO_METHOD[macro], header))
    return entries

def topological_sort(entries: list[ClassEntry]) -> list[ClassEntry]:
    known = {e.class_name for e in entries}
    result, visited = [], set()
    def visit(entry: ClassEntry):
        if entry.class_name in visited: return
        if entry.parent_name in known:
            parent_entry = next(e for e in entries if e.class_name == entry.parent_name)
            visit(parent_entry)
        visited.add(entry.class_name); result.append(entry)
    for e in entries: visit(e)
    return result

# --- Updated Generation Logic ---

def write_if_changed(file_path: Path, new_content: str) -> bool:
    """Writes content to file only if it differs from current content to preserve mtime."""
    if file_path.exists():
        existing_content = file_path.read_text(encoding="utf-8")
        if existing_content == new_content:
            return False  # No change, skip write

    file_path.write_text(new_content, encoding="utf-8")
    return True

def make_relative_include(header: Path, core_path: Path) -> str:
    try: return header.relative_to(core_path).as_posix()
    except ValueError: return header.as_posix()

def generate_header(subfolder_name: str) -> str:
    func_name = f"register_{subfolder_name}_types"
    return (f"{GENERATED_NOTICE}#pragma once\n\nnamespace feather {{\n\n"
            f"void {func_name}();\n\n}} // namespace feather\n")

def generate_cpp(subfolder_name: str, entries: list[ClassEntry], core_path: Path) -> str:
    func_name = f"register_{subfolder_name}_types"
    gen_header = f"register_{subfolder_name}_types.gen.h"
    seen_headers = set()
    ordered_headers = []
    for e in entries:
        if e.source_header not in seen_headers:
            seen_headers.add(e.source_header)
            ordered_headers.append(make_relative_include(e.source_header, core_path))

    lines = [GENERATED_NOTICE, f'#include "{gen_header}"', '#include <core/main/class_db.h>', ""]
    for inc in ordered_headers: lines.append(f'#include "{inc}"')
    lines += ["", "namespace feather {", "", f"void {func_name}() {{"]
    if entries:
        for e in entries: lines.append(f"\tClassDB::{e.method}<{e.class_name}>();")
    else:
        lines.append("\t// No reflected classes found in this module.")
    lines += ["}", "", "} // namespace feather", ""]
    return "\n".join(lines)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--core-path", type=Path, default=Path("./core"))
    args = parser.parse_args()

    core_path = args.core_path.resolve()
    if not core_path.is_dir():
        print(f"[ERROR] core path not found: {core_path}", file=sys.stderr); sys.exit(1)

    for subfolder_name in TOP_SUBFOLDERS:
        subfolder = core_path / subfolder_name
        if not subfolder.is_dir(): continue

        raw_entries = scan_folder(subfolder)
        sorted_entries = topological_sort(raw_entries)

        header_text = generate_header(subfolder_name)
        cpp_text = generate_cpp(subfolder_name, sorted_entries, core_path)

        out_h = subfolder / f"register_{subfolder_name}_types.gen.h"
        out_cpp = subfolder / f"register_{subfolder_name}_types.gen.cpp"

        # Apply the "Silent Write" check
        h_changed = write_if_changed(out_h, header_text)
        cpp_changed = write_if_changed(out_cpp, cpp_text)

        status = []
        if h_changed: status.append(f"updated {out_h.name}")
        if cpp_changed: status.append(f"updated {out_cpp.name}")

        if status:
            print(f"  [{subfolder_name}] " + ", ".join(status))
        else:
            print(f"  [{subfolder_name}] up to date.")

    print("Done.")

if __name__ == "__main__":
    main()