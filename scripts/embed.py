#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import argparse
import re
import sys
from pathlib import Path

INCLUDE_REGEX = re.compile(r'#\s*include "([^"]+)"')
EMBED_IGNORE_START_REGEX = re.compile(r"/\*\s*cplib_embed_ignore start\s*\*/")
EMBED_IGNORE_END_REGEX = re.compile(r"/\*\s*cplib_embed_ignore end\s*\*/")
IWYU_PRAGMA_REGEX = re.compile(r"//\s*IWYU pragma:.*|/\*\s*IWYU pragma:.*\*/")
DEFAULT_IGNORE_HEADERS = {"regex.h"}


def process_file(
    file_path: Path,
    include_paths: list[Path],
    processed_files: set[Path],
    ignore_headers: set[str],
) -> list[str]:
    """
    Recursively process a header file, replacing its local #includes with file content.
    """
    if file_path in processed_files:
        return []  # Act like #pragma once to prevent circular inclusion.

    processed_files.add(file_path)

    try:
        content = file_path.read_text(encoding="utf-8")
    except FileNotFoundError:
        print(f"Error: Could not find file {file_path}", file=sys.stderr)
        sys.exit(1)

    output_lines = []
    in_ignore_block = False

    for line in content.splitlines():
        # Handle start/end of ignore blocks
        if EMBED_IGNORE_START_REGEX.search(line):
            in_ignore_block = True
            continue

        if EMBED_IGNORE_END_REGEX.search(line):
            in_ignore_block = False
            continue

        if in_ignore_block:
            continue

        # Remove IWYU pragmas
        line = IWYU_PRAGMA_REGEX.sub("", line)

        # Handle #include directives
        match = INCLUDE_REGEX.search(line)
        if match:
            header_name = match.group(1)
            if header_name in ignore_headers:
                output_lines.append(line)  # Keep this #include
                continue

            # Find the included file in the search paths
            found = False
            # First, check the directory of the current file
            search_paths = [file_path.parent] + include_paths
            for include_path in search_paths:
                included_file = (include_path / header_name).resolve()
                if included_file.exists():
                    # Recurse
                    output_lines.append(f"// --- Start embedded: {header_name} ---")
                    output_lines += process_file(
                        included_file, include_paths, processed_files, ignore_headers
                    )
                    output_lines.append(f"// --- End embedded: {header_name} ---")
                    found = True
                    break

            if not found:
                print(
                    f"Warning: Included file '{header_name}' not found. Keeping original #include.",
                    file=sys.stderr,
                )
                output_lines.append(line)
        else:
            output_lines.append(line)

    return output_lines


def main():
    parser = argparse.ArgumentParser(
        description="Embed header files into a single header."
    )
    parser.add_argument(
        "main_header", type=Path, help="The main header file to process."
    )
    parser.add_argument(
        "-I",
        "--include-dir",
        type=Path,
        action="append",
        dest="include_dirs",
        default=[],
        help="Directories to search for included files.",
    )
    args = parser.parse_args()

    main_header = args.main_header.resolve()
    include_paths = [p.resolve() for p in args.include_dirs]

    ignore_headers = set(DEFAULT_IGNORE_HEADERS)

    processed_files = set()
    final_lines = process_file(
        main_header, include_paths, processed_files, ignore_headers
    )

    sys.stdout.buffer.write(("\n".join(final_lines) + "\n").encode("utf-8"))


if __name__ == "__main__":
    main()
