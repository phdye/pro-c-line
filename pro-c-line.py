#!/usr/bin/env python3

"""
USAGE:
  pc-line-mapper <line_number> <pc_file> <c_file> [--show[=N]] [--debug]

Given a C line number, maps it back to the responsible Pro*C (.pc) line number.
Uses diff-based alignment to find inserted/replaced regions.

Options:
  -h --help         Show this screen.
  --version         Show version.
  --show[=N]        Show N lines of context (default 3).
  --debug           Print internal diff line mappings.
"""

import argparse
import difflib
from pathlib import Path
from typing import Optional, List, Dict

def parse_arguments():
    parser = argparse.ArgumentParser(description="Map C line number to responsible Pro*C line.")
    parser.add_argument("line_number", type=int, help="Line number in the generated .c file")
    parser.add_argument("pc_file", type=Path, help="Original .pc source file")
    parser.add_argument("c_file", type=Path, help="PROC-generated .c file")
    parser.add_argument("--show", nargs="?", const=3, type=int, help="Show N lines of context (default 3)")
    parser.add_argument("--debug", action="store_true", help="Print debug info about line mappings")
    return parser.parse_args()

def build_line_map(pc_lines, c_lines, debug=False):
    matcher = difflib.SequenceMatcher(None, pc_lines, c_lines, autojunk=False)
    c_to_pc = {}
    for tag, i1, i2, j1, j2 in matcher.get_opcodes():
        if debug:
            print(f"{tag:>7}: pc[{i1}:{i2}] -> c[{j1}:{j2}]")
        if tag == 'equal':
            for offset in range(j2 - j1):
                c_to_pc[j1 + offset + 1] = i1 + offset + 1
        elif tag == 'replace':
            for k in range(j1, j2):
                c_to_pc[k + 1] = i1 + 1 if i1 < len(pc_lines) else len(pc_lines)
        elif tag == 'insert':
            for k in range(j1, j2):
                c_to_pc[k + 1] = None
        elif tag == 'delete':
            continue
        # Insert dummy span for early C lines before any mapped region
    first_c_line = min(c_to_pc.keys()) if c_to_pc else 1
    for i in range(1, first_c_line):
        c_to_pc[i] = 0
    return c_to_pc

def map_c_line_to_pc_line(c_line, pc_lines, c_lines, debug=False):
    if c_line < 1:
        return "Line number must be positive."
    if c_line > len(c_lines):
        return f"Line {c_line} is beyond the end of the C code."
    line_map = build_line_map(pc_lines, c_lines, debug=debug)
    if c_line not in line_map:
        first_c_line = min(line_map.keys()) if line_map else 1
        return "0" if c_line < first_c_line else None
    if line_map[c_line] is None:
        return None
    return line_map[c_line]

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
    debug = args.debug

    pc_lines = pc_path.read_text(encoding="utf-8").splitlines()
    c_lines = c_path.read_text(encoding="utf-8").splitlines()

    pc_line = map_c_line_to_pc_line(c_line, pc_lines, c_lines, debug=debug)

    if isinstance(pc_line, str):
        if pc_line == "0":
            print(f"C line {c_line} maps to {pc_path.name}:0")
        else:
            print(pc_line)
    elif isinstance(pc_line, int):
        print(f"C line {c_line} maps to {pc_path.name}:{pc_line}")
        if show_context is not None and pc_line > 0:
            print("\nContext:")
            print_context_lines(pc_path, pc_line, show_context)
    else:
        print(f"Unable to map C line {c_line} to {pc_path.name}")

if __name__ == "__main__":
    main()
