#!/usr/bin/env python3
"""Check that every C/C++ source file in the project is clang-formatted."""

import subprocess
import sys

from format_project import BATCH_SIZE, ROOT, collect_files, find_clang_format, parse_args


def main() -> int:
    args = parse_args(__doc__)
    clang_format = find_clang_format(args.clang_format)
    version = subprocess.run([clang_format, "--version"], capture_output=True, text=True).stdout.strip()
    files = collect_files()
    print(f"Checking {len(files)} files with {version}")
    unformatted = set()
    for i in range(0, len(files), BATCH_SIZE):
        batch = files[i:i + BATCH_SIZE]
        proc = subprocess.run(
            [clang_format, "--dry-run", "--Werror", "--style=file"] + batch,
            cwd=ROOT, capture_output=True, text=True,
        )
        # Diagnostics look like "path:line:col: error: ...", so match each file by prefix.
        stderr = proc.stderr.replace("\\", "/")
        unformatted.update(f for f in batch if f"{f}:" in stderr)
        if proc.returncode != 0 and not proc.stderr.strip():
            print(proc.stdout, file=sys.stderr)
            return proc.returncode
    if unformatted:
        print(f"\n{len(unformatted)} file(s) are not clang-formatted:", file=sys.stderr)
        for f in sorted(unformatted):
            print(f"  {f}", file=sys.stderr)
        print("\nRun `python tools/format/format_project.py` to fix them.", file=sys.stderr)
        return 1
    print("All files are correctly formatted.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
