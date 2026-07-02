#!/usr/bin/env python3
"""
Generate C++ header files with extern declarations and a single .cpp file with
#embed directives for resources in raw_resources folder.
Uses C++26 #embed feature to embed binary data directly into the compiled binary.
"""

import argparse
import os
import sys
from pathlib import Path


def sanitize_variable_name(path: Path, base_path: Path) -> str:
    """
    Convert a file path to a valid C++ variable name.
    Example: raw_resources/shaders/pbr_forward.slang -> pbr_forward_slang
    """
    rel_path = path.relative_to(base_path)
    name = str(rel_path).replace(os.sep, '_').replace('/', '_').replace('.', '_')
    name = name.lstrip('_')
    return name


def generate_header(source_file: Path, output_file: Path, base_path: Path, dry_run: bool = False) -> str:
    """
    Generate a C++ header file with an extern declaration for the given source file.
    Returns the variable name used.
    """
    var_name = sanitize_variable_name(source_file, base_path)

    header_content = f"""#pragma once
#include <cstddef>

//extern const int {var_name}_storage[];
extern const std::size_t {var_name}_size;
extern const std::byte* {var_name};
"""

    if dry_run:
        print(f"[dry-run] Would generate header: {output_file}")
        return var_name

    output_file.parent.mkdir(parents=True, exist_ok=True)
    with open(output_file, 'w', encoding='utf-8', newline='\n') as f:
        f.write(header_content)

    print(f"Generated header: {output_file}")
    return var_name


def generate_cpp(entries: list[tuple[str, str, Path]], output_file: Path, base_path: Path, dry_run: bool = False) -> None:
    """
    Generate a single .cpp file containing all #embed definitions.

    entries: list of (var_name, rel_embed_path, source_file) tuples
    base_path: the raw_resources root — include paths are relative to this
    """
    lines = [
        "#include <cstddef>",
        "",
    ]

    # Include paths relative to base_path (i.e. raw_resources), which should
    # be added as an include directory in the build system.
    for var_name, rel_embed_path, source_file in entries:
        header_name = f"{source_file.name}.gen.h"
        rel_to_base = source_file.parent.relative_to(base_path.parent)
        include_path = (rel_to_base / header_name).as_posix()
        lines.append(f'#include <{include_path}>')

    lines.append("")

    for var_name, rel_embed_path, source_file in entries:
        lines += [
            f"const int {var_name}_storage[] = {{",
            f'#embed "{rel_embed_path}"',
            "};",
            f"const std::size_t {var_name}_size = sizeof({var_name}_storage);",
            f"const std::byte* {var_name} = reinterpret_cast<const std::byte*>({var_name}_storage);",
            "",
        ]

    cpp_content = "\n".join(lines)

    if dry_run:
        print(f"[dry-run] Would generate cpp: {output_file}")
        return

    output_file.parent.mkdir(parents=True, exist_ok=True)
    with open(output_file, 'w', encoding='utf-8', newline='\n') as f:
        f.write(cpp_content)

    print(f"Generated cpp:    {output_file}")


def clean_generated(raw_resources_dir: Path, project_root: Path, dry_run: bool = False) -> None:
    """Remove all .gen.h files and the generated resources.cpp."""
    removed_count = 0

    for gen_file in sorted(raw_resources_dir.rglob('*.gen.h')):
        if dry_run:
            print(f"[dry-run] Would remove: {gen_file}")
        else:
            gen_file.unlink()
            print(f"Removed: {gen_file}")
        removed_count += 1

    cpp_out = project_root / 'raw_resources' / 'resources.gen.cpp'
    if cpp_out.exists():
        if dry_run:
            print(f"[dry-run] Would remove: {cpp_out}")
        else:
            cpp_out.unlink()
            print(f"Removed: {cpp_out}")
        removed_count += 1

    print()
    print("Summary:")
    if dry_run:
        print(f"  Would remove: {removed_count} file(s)")
    else:
        print(f"  Removed: {removed_count} file(s)")


def should_process_file(file_path: Path) -> bool:
    """
    Determine if a file should have a header generated for it.
    Skips .h/.cpp files and other build artifacts.
    """
    if file_path.suffix in ['.h', '.hpp', '.hxx', '.cpp', '.cxx', '.cc']:
        return False
    if file_path.name.startswith('.'):
        return False
    skip_extensions = ['.o', '.obj', '.exe', '.dll', '.so', '.dylib', '.a', '.lib']
    if file_path.suffix.lower() in skip_extensions:
        return False
    return True


def main():
    parser = argparse.ArgumentParser(
        description="Generate C++ headers (extern declarations) and a single .cpp "
                    "(#embed definitions) for all files in raw_resources."
    )
    parser.add_argument(
        '--dry-run', action='store_true',
        help="Print what would be done without writing or deleting any files."
    )
    parser.add_argument(
        '--clean', action='store_true',
        help="Remove all generated .gen.h and resources.gen.cpp files."
    )
    parser.add_argument(
        '--force', action='store_true',
        help="Overwrite existing generated files instead of skipping them."
    )
    args = parser.parse_args()

    script_dir = Path(__file__).parent.resolve()
    project_root = script_dir.parent
    raw_resources_dir = project_root / 'raw_resources'

    if not raw_resources_dir.exists():
        print(f"Error: raw_resources directory not found at {raw_resources_dir}", file=sys.stderr)
        sys.exit(1)

    if args.clean:
        print(f"Cleaning generated files under: {raw_resources_dir}")
        if args.dry_run:
            print("[dry-run mode]")
        print()
        clean_generated(raw_resources_dir, project_root, dry_run=args.dry_run)
        return

    print(f"Scanning: {raw_resources_dir}")
    print(f"Project root: {project_root}")
    if args.dry_run:
        print("[dry-run mode]")
    elif args.force:
        print("[force mode — overwriting existing files]")
    print()

    generated_count = 0
    skipped_count = 0
    # (var_name, rel_embed_path_from_cpp, source_file)
    cpp_entries: list[tuple[str, str, Path]] = []

    cpp_out = raw_resources_dir / 'resources.gen.cpp'

    for source_file in sorted(raw_resources_dir.rglob('*')):
        if source_file.is_dir():
            continue
        if not should_process_file(source_file):
            skipped_count += 1
            continue

        output_file = source_file.parent / f"{source_file.name}.gen.h"
        var_name = sanitize_variable_name(source_file, raw_resources_dir)

        if not args.force and not args.dry_run and output_file.exists():
            skipped_count += 1
        else:
            try:
                generate_header(source_file, output_file, raw_resources_dir, dry_run=args.dry_run)
                generated_count += 1
            except Exception as e:
                print(f"Error generating header for {source_file}: {e}", file=sys.stderr)
                continue

        # Relative path from the .cpp file to the source file for #embed
        try:
            rel_embed = os.path.relpath(source_file, cpp_out.parent).replace('\\', '/')
        except ValueError:
            rel_embed = str(source_file).replace('\\', '/')

        cpp_entries.append((var_name, rel_embed, source_file))

    # Generate the single .cpp with all definitions
    if cpp_entries:
        if not args.force and not args.dry_run and cpp_out.exists():
            print(f"Skipped (cached): {cpp_out}")
        else:
            try:
                generate_cpp(cpp_entries, cpp_out, raw_resources_dir, dry_run=args.dry_run)
            except Exception as e:
                print(f"Error generating cpp file: {e}", file=sys.stderr)

    print()
    print("Summary:")
    if args.dry_run:
        print(f"  Would generate: {generated_count} header(s) + 1 .cpp file")
    else:
        print(f"  Generated: {generated_count} header(s) + 1 .cpp file")
    print(f"  Skipped: {skipped_count} file(s)")


if __name__ == '__main__':
    main()