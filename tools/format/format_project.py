#!/usr/bin/env python3
"""Apply clang-format to every C/C++ source file in the project."""

import argparse
import os
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
EXTENSIONS = (".c", ".cc", ".cpp", ".cxx", ".cppm", ".ixx", ".h", ".hpp", ".hxx", ".inl")
EXCLUDED_DIRS = ("thirdparty/", "build/")
EXCLUDED_SUFFIXES = (".gen.h", ".gen.cpp")
BATCH_SIZE = 50  # keep command lines short enough for Windows


def find_clang_format(explicit: str | None) -> str:
    exe = explicit or os.environ.get("CLANG_FORMAT") or shutil.which("clang-format")
    if not exe:
        sys.exit("error: clang-format not found (set CLANG_FORMAT or pass --clang-format)")
    return exe


def collect_files() -> list[str]:
    out = subprocess.run(
        ["git", "ls-files", "--cached", "--others", "--exclude-standard"],
        cwd=ROOT, check=True, capture_output=True, text=True,
    ).stdout
    return [
        f for f in out.splitlines()
        if f.endswith(EXTENSIONS)
        and not f.startswith(EXCLUDED_DIRS)
        and not f.endswith(EXCLUDED_SUFFIXES)
        and (ROOT / f).is_file()
    ]


def run_batched(cmd: list[str], files: list[str]) -> int:
    result = 0
    for i in range(0, len(files), BATCH_SIZE):
        result |= subprocess.run(cmd + files[i:i + BATCH_SIZE], cwd=ROOT).returncode
    return result


def parse_args(description: str) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=description)
    parser.add_argument("--clang-format", help="path to the clang-format executable")
    return parser.parse_args()


def main() -> int:
    args = parse_args(__doc__)
    clang_format = find_clang_format(args.clang_format)
    files = collect_files()
    print(f"Formatting {len(files)} files with {clang_format}")
    return run_batched([clang_format, "-i", "--style=file"], files)


if __name__ == "__main__":
    sys.exit(main())
