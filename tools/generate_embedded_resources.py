#!/usr/bin/env python3
"""
Generate C++ header files with #embed directives for resources in raw_resources folder.
Uses C++26 #embed feature to embed binary data directly into the compiled binary.
"""

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


def generate_header(source_file: Path, output_file: Path, base_path: Path) -> None:
    """
    Generate a C++ header file with #embed directive for the given source file.

    Args:
        source_file: Path to the source file to embed
        output_file: Path where the .h file will be generated
        base_path: Base path for calculating relative paths
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

static const char {var_name}[] = {{
#embed "{rel_path}"
\t, '\\0'
}};
"""

    # Write header file
    output_file.parent.mkdir(parents=True, exist_ok=True)
    with open(output_file, 'w', encoding='utf-8', newline='\n') as f:
        f.write(header_content)

    print(f"Generated: {output_file}")


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
    # Determine script directory and project root
    script_dir = Path(__file__).parent.resolve()
    project_root = script_dir.parent
    raw_resources_dir = project_root / 'raw_resources'

    # Check if raw_resources directory exists
    if not raw_resources_dir.exists():
        print(f"Error: raw_resources directory not found at {raw_resources_dir}", file=sys.stderr)
        sys.exit(1)

    print(f"Scanning: {raw_resources_dir}")
    print(f"Project root: {project_root}")
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
        output_file = source_file.parent / f"{source_file.name}.h"
        # skip existing files
        if (os.path.exists(output_file)):
            continue

        # Generate the header
        try:
            generate_header(source_file, output_file, raw_resources_dir)
            generated_count += 1
        except Exception as e:
            print(f"Error generating header for {source_file}: {e}", file=sys.stderr)

    # Print summary
    print()
    print(f"Summary:")
    print(f"  Generated: {generated_count} header(s)")
    print(f"  Skipped: {skipped_count} file(s)")


if __name__ == '__main__':
    main()
