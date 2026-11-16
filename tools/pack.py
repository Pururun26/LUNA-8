#!/usr/bin/env python3
"""pack.py — LUNA-8 packer.

Bundles multiple .lua modules into a single game.lua.

Usage:
    python tools/pack.py main.lua
        -> game.lua (next to main.lua)

    python tools/pack.py main.lua -o build/game.lua
        -> build/game.lua

Include format (one line):
    --[[player.lua]]
    -- [[enemies.lua]]
    --[[   utils.lua   ]]

Path is relative to the current file's directory.
Include order follows source order.
Cycles are detected via realpath stack.
"""

import argparse
import os
import re
import sys

MAX_DEPTH = 32
INCLUDE_RE = re.compile(r'^--\s*\[\[\s*(.+\.lua)\s*\]\]\s*$')


def parse_include(line):
    m = INCLUDE_RE.match(line.strip())
    return m.group(1) if m else None


def pack_file(path, depth, stack, out):
    if depth > MAX_DEPTH:
        raise RuntimeError(
            f"include depth exceeded ({MAX_DEPTH}): "
            f"{' -> '.join(stack)}"
        )

    real = os.path.realpath(path)
    if real in stack:
        raise RuntimeError(
            f"circular include: {' -> '.join(stack + [real])}"
        )
    stack.append(real)

    base_dir = os.path.dirname(path) or "."
    fname = os.path.basename(path)

    try:
        with open(path, "r", encoding="utf-8", newline="") as f:
            lines = f.readlines()
    except FileNotFoundError:
        raise RuntimeError(f"file not found: '{path}'")
    except OSError as e:
        raise RuntimeError(f"cannot read '{path}': {e}")

    if depth > 0:
        out.append(f"-- >>> {fname}")

    for raw in lines:
        line = raw.rstrip("\r\n")
        name = parse_include(line)
        if name is not None:
            inc_path = os.path.join(base_dir, name)
            pack_file(inc_path, depth + 1, stack, out)
        else:
            out.append(line)

    if depth > 0:
        out.append(f"-- <<< {fname}")

    stack.pop()


def main():
    ap = argparse.ArgumentParser(description="LUNA-8 packer: main.lua -> game.lua")
    ap.add_argument("main", help="main file with --[[.lua]] includes")
    ap.add_argument("-o", "--output",
                    help="output file (default: game.lua next to main)")
    ap.add_argument("-q", "--quiet", action="store_true",
                    help="suppress summary line")
    args = ap.parse_args()

    out = []
    try:
        pack_file(args.main, 0, [], out)
    except RuntimeError as e:
        print(f"pack.py: {e}", file=sys.stderr)
        return 1

    out_path = args.output
    if not out_path:
        out_path = "game.lua"

    try:
        os.makedirs(os.path.dirname(os.path.abspath(out_path)), exist_ok=True)
        with open(out_path, "w", encoding="utf-8", newline="\n") as f:
            for line in out:
                f.write(line + "\n")
    except OSError as e:
        print(f"pack.py: cannot write '{out_path}': {e}", file=sys.stderr)
        return 1

    if not args.quiet:
        print(f"packed {args.main} -> {out_path} ({len(out)} lines)")
    return 0


if __name__ == "__main__":
    sys.exit(main())