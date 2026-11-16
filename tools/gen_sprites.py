#!/usr/bin/env python3
"""gen_sprite.py — PNG → sprdata() строка для LUNA-8.

Usage:
    python tools/gen_sprite.py sprite.png
    python tools/gen_sprite.py sprite.png --lua 1
    python tools/gen_sprite.py sprite.png --lua 5 --size 16x16
    python tools/gen_sprite.py sprite.png --colorkey 0
    python tools/gen_sprite.py sprite.png --palette "2f2f2f,ff0000,..."

Палитра по умолчанию (DEFAULT_PALETTE):
    0 black, 1 red, 2 green, 3 blue, 4 cyan, 5 magenta, 6 yellow, 7 white

Каждый пиксель PNG с alpha < 128 → colorkey (по умолчанию 0).
Остальные → ближайший по RGB цвет из палитры (Euclidean distance).
"""

import sys
import argparse
import math
from PIL import Image

DEFAULT_PALETTE = [
    (0x00, 0x00, 0x00),  # 0
    (0xFF, 0x00, 0x00),  # 1
    (0x00, 0xFF, 0x00),  # 2
    (0x00, 0x00, 0xFF),  # 3
    (0x00, 0xFF, 0xFF),  # 4
    (0xFF, 0x00, 0xFF),  # 5
    (0xFF, 0xFF, 0x00),  # 6
    (0xFF, 0xFF, 0xFF),  # 7
]


def parse_palette(s):
    if not s:
        return DEFAULT_PALETTE
    colors = []
    for h in s.split(','):
        h = h.strip().lstrip('#')
        if len(h) != 6:
            raise ValueError(f"bad hex color: {h!r} (need 6 hex chars)")
        try:
            r = int(h[0:2], 16)
            g = int(h[2:4], 16)
            b = int(h[4:6], 16)
        except ValueError:
            raise ValueError(f"bad hex color: {h!r}")
        colors.append((r, g, b))
    if len(colors) > 8:
        raise ValueError(f"palette has {len(colors)} colors, max is 8")
    return colors


def nearest_index(rgb, palette):
    r, g, b = rgb
    best_i = 0
    best_d = 1 << 30
    for i, (pr, pg, pb) in enumerate(palette):
        dr = r - pr
        dg = g - pg
        db = b - pb
        d = dr * dr + dg * dg + db * db
        if d < best_d:
            best_d = d
            best_i = i
    return best_i, best_d


def main():
    ap = argparse.ArgumentParser(description="PNG → LUNA-8 sprdata()")
    ap.add_argument("png", help="input PNG")
    ap.add_argument("--lua", type=int, default=None, metavar="ID",
                    help="wrap in sprdata(ID, [[...]])")
    ap.add_argument("--colorkey", type=int, default=0,
                    help="palette index for transparent pixels (default 0)")
    ap.add_argument("--palette", default=None,
                    help="custom palette: hex,hex,... (max 8)")
    ap.add_argument("--size", default=None, metavar="WxH",
                    help="expected size, e.g. 8x8 or 16x16")
    ap.add_argument("--quiet", action="store_true",
                    help="no info on stderr")
    args = ap.parse_args()

    try:
        palette = parse_palette(args.palette)
    except ValueError as e:
        print(f"error: {e}", file=sys.stderr)
        return 1

    try:
        img = Image.open(args.png).convert('RGBA')
    except Exception as e:
        print(f"error: cannot open {args.png}: {e}", file=sys.stderr)
        return 1

    w, h = img.size

    if args.size:
        parts = args.size.lower().split('x')
        if len(parts) != 2:
            print(f"error: --size expects WxH, got {args.size!r}", file=sys.stderr)
            return 1
        try:
            ew, eh = int(parts[0]), int(parts[1])
        except ValueError:
            print(f"error: --size expects numbers, got {args.size!r}", file=sys.stderr)
            return 1
        if (w, h) != (ew, eh):
            print(f"error: PNG is {w}x{h}, expected {ew}x{eh}", file=sys.stderr)
            return 1

    if w > 64 or h > 64:
        print(f"warning: sprite is {w}x{h}, LUNA-8 max is 64x64", file=sys.stderr)

    px = img.load()
    rows = []
    max_dist_sq = 0

    for y in range(h):
        line = []
        for x in range(w):
            r, g, b, a = px[x, y]
            if a < 128:
                line.append(str(args.colorkey))
            else:
                idx, d = nearest_index((r, g, b), palette)
                line.append(str(idx))
                if d > max_dist_sq:
                    max_dist_sq = d
        rows.append(''.join(line))

    body = '\n'.join(rows)

    # Готовый Lua-фрагмент
    if args.lua is not None:
        if w == 8 and h == 8:
            out = f'sprdata({args.lua}, [[\n{body}\n]])'
        else:
            out = f'sprdata({args.lua}, [[\n{body}\n]], {w}, {h})'
    else:
        out = body

    print(out)

    if not args.quiet:
        err = math.sqrt(max_dist_sq)
        info = f"# {w}x{h}, colorkey={args.colorkey}, max color error {err:.0f}"
        if err > 64:
            info += "  ⚠ (far from palette — check colors)"
        print(info, file=sys.stderr)

    return 0


if __name__ == '__main__':
    sys.exit(main())