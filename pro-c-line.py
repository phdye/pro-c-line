#!/usr/bin/env python3

"""
USAGE:
  pc-line-mapper <line_number> <pc_file> <c_file> [--show[=N]]

Given a C line number, maps it back to the responsible Pro*C (.pc) line number.
Handles EXEC SQL INCLUDE and preprocessing complexities.

Options:
  -h --help         Show this screen.
  --version         Show version.
  --show[=N]        Show N lines of context (default 3).
"""

import argparse
import re
from pathlib import Path
from typing import Optional

def parse_arguments():
    parser = argparse.ArgumentParser(description="Map C line number to responsible Pro*C line.")
    parser.add_argument("line_number", type=int, help="Line number in the generated .c file")
    parser.add_argument("pc_file", type=Path, help="Original .pc source file")
    parser.add_argument("c_file", type=Path, help="PROC-generated .c file")
    parser.add_argument("--show", nargs="?", const=3, type=int, help="Show N lines of context (default 3)")
    return parser.parse_args()

def parse_c_file_for_line_mapping(c_path: Path):
    """
    Parses the .c file and returns a list of tuples:
    (c_line_number, original_source_file, original_source_line)
    based on #line or # <lineno> "filename" directives.
    """
    mappings = []
    current_file = None
    current_src_line = None
    with c_path.open("r", encoding="utf-8") as f:
        for i, line in enumerate(f, 1):
            match = re.match(r'#\s*(\d+)\s+"([^"]+)"', line)
            if match:
                current_src_line = int(match.group(1))
                current_file = match.group(2)
            mappings.append((i, current_file, current_src_line))
            if current_src_line is not None:
                current_src_line += 1
    return mappings

def find_pc_line_number(c_line: int, mappings: list[tuple[int, Optional[str], Optional[int]]], target_pc: Path) -> Optional[int]:
    """
    Given a line in .c and the mappings, walk backwards to find the most
    recent #line directive mapping it back to the .pc file.
    """
    for i in range(c_line - 1, -1, -1):
        mapped_c_line, src_file, src_line = mappings[i]
        if src_file and Path(src_file).resolve() == target_pc.resolve():
            return src_line + (c_line - mapped_c_line)
    return None

def print_context_lines(file: Path, center_line: int, context: int):
    lines = file.read_text(encoding="utf-8").splitlines()
    start = max(center_line - context - 1, 0)
    end = min(center_line + context, len(lines))
    for i in range(start, end):
        marker = "-->" if i == center_line - 1 else "   "
        print(f"{marker} {i + 1:5}: {lines[i]}")

def main():
    args = parse_arguments()
    c_line = args.line_number
    pc_path = args.pc_file
    c_path = args.c_file
    show_context = args.show

    mappings = parse_c_file_for_line_mapping(c_path)
    pc_line = find_pc_line_number(c_line, mappings, pc_path)

    if pc_line is not None:
        print(f"C line {c_line} maps to {pc_path.name}:{pc_line}")
        if show_context is not None:
            print("\nContext:")
            print_context_lines(pc_path, pc_line, show_context)
    else:
        print(f"Unable to map C line {c_line} to {pc_path.name}")

if __name__ == "__main__":
    main()
