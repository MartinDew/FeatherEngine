#!/usr/bin/env python3
"""
Generate C++ header files with #embed directives for resources in raw_resources folder.
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
    # Get relative path from base
    rel_path = path.relative_to(base_path)

    # Replace path separators and dots with underscores
    name = str(rel_path).replace(os.sep, '_').replace('/', '_').replace('.', '_')

    # Remove leading underscores
    name = name.lstrip('_')

    return name


def generate_header(source_file: Path, output_file: Path, base_path: Path, dry_run: bool = False) -> None:
    """
    Generate a C++ header file with #embed directive for the given source file.

    Args:
        source_file: Path to the source file to embed
        output_file: Path where the .h file will be generated
        base_path: Base path for calculating relative paths
        dry_run: If True, print what would be done without writing files
    """
    # Calculate relative path from output file to source file
    try:
        rel_path = os.path.relpath(source_file, output_file.parent)
        # Convert Windows backslashes to forward slashes for #embed
        rel_path = rel_path.replace('\\', '/')
    except ValueError:
        # Different drives on Windows, use absolute path
        rel_path = str(source_file).replace('\\', '/')

    # Generate variable name
    var_name = sanitize_variable_name(source_file, base_path)

    # Generate header content
    header_content = f"""#pragma once
#ifdef _WIN32
static const char {var_name}[] = {{
#else
static const unsigned char {var_name}[] = {{
#endif
#embed "{rel_path}"
\t, '\\0'
}};
"""

    if dry_run:
        print(f"[dry-run] Would generate: {output_file}")
        return

    # Write header file
    output_file.parent.mkdir(parents=True, exist_ok=True)
    with open(output_file, 'w', encoding='utf-8', newline='\n') as f:
        f.write(header_content)

    print(f"Generated: {output_file}")


def clean_generated(raw_resources_dir: Path, dry_run: bool = False) -> None:
    """Remove all .gen.h files under raw_resources_dir."""
    removed_count = 0
    for gen_file in sorted(raw_resources_dir.rglob('*.gen.h')):
        if dry_run:
            print(f"[dry-run] Would remove: {gen_file}")
        else:
            gen_file.unlink()
            print(f"Removed: {gen_file}")
        removed_count += 1

    print()
    print(f"Summary:")
    if dry_run:
        print(f"  Would remove: {removed_count} file(s)")
    else:
        print(f"  Removed: {removed_count} file(s)")


def should_process_file(file_path: Path) -> bool:
    """
    Determine if a file should have a header generated for it.
    Skips .h files and other build artifacts.
    """
    # Skip header files themselves
    if file_path.suffix in ['.h', '.hpp', '.hxx']:
        return False

    # Skip hidden files
    if file_path.name.startswith('.'):
        return False

    # Skip common build artifacts
    skip_extensions = ['.o', '.obj', '.exe', '.dll', '.so', '.dylib', '.a', '.lib']
    if file_path.suffix.lower() in skip_extensions:
        return False

    return True


def main():
    """Main entry point for the script."""
    parser = argparse.ArgumentParser(
        description="Generate C++ headers with #embed directives for raw_resources files."
    )
    parser.add_argument(
        '--dry-run', action='store_true',
        help="Print what would be done without writing or deleting any files."
    )
    parser.add_argument(
        '--clean', action='store_true',
        help="Remove all generated .gen.h files instead of generating them."
    )
    args = parser.parse_args()

    # Determine script directory and project root
    script_dir = Path(__file__).parent.resolve()
    project_root = script_dir.parent
    raw_resources_dir = project_root / 'raw_resources'

    # Check if raw_resources directory exists
    if not raw_resources_dir.exists():
        print(f"Error: raw_resources directory not found at {raw_resources_dir}", file=sys.stderr)
        sys.exit(1)

    if args.clean:
        print(f"Cleaning generated headers under: {raw_resources_dir}")
        if args.dry_run:
            print("[dry-run mode]")
        print()
        clean_generated(raw_resources_dir, dry_run=args.dry_run)
        return

    print(f"Scanning: {raw_resources_dir}")
    print(f"Project root: {project_root}")
    if args.dry_run:
        print("[dry-run mode]")
    print()

    # Track statistics
    generated_count = 0
    skipped_count = 0

    # Recursively find all files in raw_resources
    for source_file in sorted(raw_resources_dir.rglob('*')):
        # Skip directories
        if source_file.is_dir():
            continue

        # Skip files that shouldn't be processed
        if not should_process_file(source_file):
            skipped_count += 1
            continue

        # Generate output path (same location as source, with .h appended)
        output_file = source_file.parent / f"{source_file.name}.gen.h"
        # skip existing files
        if not args.dry_run and os.path.exists(output_file):
            continue

        # Generate the header
        try:
            generate_header(source_file, output_file, raw_resources_dir, dry_run=args.dry_run)
            generated_count += 1
        except Exception as e:
            print(f"Error generating header for {source_file}: {e}", file=sys.stderr)

    # Print summary
    print()
    print(f"Summary:")
    if args.dry_run:
        print(f"  Would generate: {generated_count} header(s)")
    else:
        print(f"  Generated: {generated_count} header(s)")
    print(f"  Skipped: {skipped_count} file(s)")


if __name__ == '__main__':
    main()
