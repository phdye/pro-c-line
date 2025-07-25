#!/usr/bin/env python3.2

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
import os

def parse_arguments():
    parser = argparse.ArgumentParser(description="Map C line number to responsible Pro*C line.")
    parser.add_argument("line_number", type=int, help="Line number in the generated .c file")
    parser.add_argument("pc_file", help="Original .pc source file")
    parser.add_argument("c_file", help="PROC-generated .c file")
    parser.add_argument("--show", nargs="?", const=3, type=int, help="Show N lines of context (default 3)")
    parser.add_argument("--debug", action="store_true", help="Print debug info about line mappings")
    return parser.parse_args()

def build_line_map(pc_lines, c_lines, debug=False):
    matcher = difflib.SequenceMatcher(None, pc_lines, c_lines, autojunk=False)
    c_to_pc = {}
    for tag, i1, i2, j1, j2 in matcher.get_opcodes():
        if debug:
            print("{:>7}: pc[{}:{}] -> c[{}:{}]".format(tag, i1, i2, j1, j2))
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
        return "Line {} is beyond the end of the C code.".format(c_line)
    line_map = build_line_map(pc_lines, c_lines, debug=debug)
    if c_line not in line_map:
        first_c_line = min(line_map.keys()) if line_map else 1
        return "0" if c_line < first_c_line else None
    if line_map[c_line] is None:
        return None
    return line_map[c_line]

def print_context_lines(filename, center_line, context):
    with open(filename, 'r', encoding='utf-8') as fh:
        lines = fh.read().splitlines()
    start = max(center_line - context - 1, 0)
    end = min(center_line + context, len(lines))
    for i in range(start, end):
        marker = "-->" if i == center_line - 1 else "   "
        print("{0} {1:5}: {2}".format(marker, i + 1, lines[i]))

def main():
    args = parse_arguments()
    c_line = args.line_number
    pc_path = args.pc_file
    c_path = args.c_file
    show_context = args.show
    debug = args.debug

    with open(pc_path, 'r', encoding='utf-8') as fh:
        pc_lines = fh.read().splitlines()
    with open(c_path, 'r', encoding='utf-8') as fh:
        c_lines = fh.read().splitlines()

    pc_line = map_c_line_to_pc_line(c_line, pc_lines, c_lines, debug=debug)

    pc_name = os.path.basename(pc_path)
    if isinstance(pc_line, str):
        if pc_line == "0":
            print("C line {} maps to {}:0".format(c_line, pc_name))
        else:
            print(pc_line)
    elif isinstance(pc_line, int):
        print("C line {} maps to {}:{}".format(c_line, pc_name, pc_line))
        if show_context is not None and pc_line > 0:
            print("\nContext:")
            print_context_lines(pc_path, pc_line, show_context)
    else:
        print("Unable to map C line {} to {}".format(c_line, pc_name))

if __name__ == "__main__":
    main()
