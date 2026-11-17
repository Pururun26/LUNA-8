#!/usr/bin/env python3
"""gen_sprite.py — PNG → sprdata() строка для LUNA-8.

Usage:
    python tools/gen_sprite.py sprite.png
    python tools/gen_sprite.py sprite.png --lua 1
    python tools/gen_sprite.py sprite.png --lua 5 --size 16x16
    python tools/gen_sprite.py sprite.png --colorkey 0
    python tools/gen_sprite.py sprite.png --palette "000000,555555,..."

Палитра по умолчанию (DEFAULT_PALETTE, 16 цветов):
    0 000000 black        1 555555 dk gray
    2 aaaaaa lt gray      3 ffffff white
    4 0000aa dk blue      5 5555ff lt blue
    6 00aa00 dk green     7 55ff55 lt green
    8 00aaaa dk cyan      9 55ffff lt cyan
    a aa0000 dk red       b ff5555 lt red
    c aa00aa dk magenta   d ff55ff lt magenta
    e aa5500 brown        f ffff55 yellow

Пиксели выводятся hex-символами 0-9a-f (нижний регистр), совместимыми
с парсером luna8_sprdata(). Каждый пиксель PNG с alpha < 128 → colorkey
(по умолчанию 0). Остальные → ближайший по RGB цвет из палитры.
"""

import sys
import argparse
import math
from PIL import Image

HEX = "0123456789abcdef"

DEFAULT_PALETTE = [
    (0x00, 0x00, 0x00),  # 0
    (0x55, 0x55, 0x55),  # 1
    (0xAA, 0xAA, 0xAA),  # 2
    (0xFF, 0xFF, 0xFF),  # 3
    (0x00, 0x00, 0xAA),  # 4
    (0x55, 0x55, 0xFF),  # 5
    (0x00, 0xAA, 0x00),  # 6
    (0x55, 0xFF, 0x55),  # 7
    (0x00, 0xAA, 0xAA),  # 8
    (0x55, 0xFF, 0xFF),  # 9
    (0xAA, 0x00, 0x00),  # 10 / a
    (0xFF, 0x55, 0x55),  # 11 / b
    (0xAA, 0x00, 0xAA),  # 12 / c
    (0xFF, 0x55, 0xFF),  # 13 / d
    (0xAA, 0x55, 0x00),  # 14 / e
    (0xFF, 0xFF, 0x55),  # 15 / f
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
    if len(colors) > 16:
        raise ValueError(f"palette has {len(colors)} colors, max is 16")
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
                    help="palette index (0..15) for transparent pixels (default 0)")
    ap.add_argument("--palette", default=None,
                    help="custom palette: hex,hex,... (max 16)")
    ap.add_argument("--size", default=None, metavar="WxH",
                    help="expected size, e.g. 8x8 or 16x16")
    ap.add_argument("--quiet", action="store_true",
                    help="no info on stderr")
    args = ap.parse_args()

    if not (0 <= args.colorkey <= 15):
        print(f"error: --colorkey must be 0..15, got {args.colorkey}",
              file=sys.stderr)
        return 1

    try:
        palette = parse_palette(args.palette)
    except ValueError as e:
        print(f"error: {e}", file=sys.stderr)
        return 1

    if args.colorkey >= len(palette):
        print(f"error: --colorkey {args.colorkey} is outside palette "
              f"of {len(palette)} colors", file=sys.stderr)
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
            print(f"error: --size expects WxH, got {args.size!r}",
                  file=sys.stderr)
            return 1
        try:
            ew, eh = int(parts[0]), int(parts[1])
        except ValueError:
            print(f"error: --size expects numbers, got {args.size!r}",
                  file=sys.stderr)
            return 1
        if (w, h) != (ew, eh):
            print(f"error: PNG is {w}x{h}, expected {ew}x{eh}",
                  file=sys.stderr)
            return 1

    if w > 64 or h > 64:
        print(f"warning: sprite is {w}x{h}, LUNA-8 max is 64x64",
              file=sys.stderr)

    px = img.load()
    rows = []
    max_dist_sq = 0
    key_char = HEX[args.colorkey]

    for y in range(h):
        line = []
        for x in range(w):
            r, g, b, a = px[x, y]
            if a < 128:
                line.append(key_char)
            else:
                idx, d = nearest_index((r, g, b), palette)
                line.append(HEX[idx])
                if d > max_dist_sq:
                    max_dist_sq = d
        rows.append(''.join(line))

    body = '\n'.join(rows)

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
        info = (f"# {w}x{h}, colorkey=0x{args.colorkey:x} (index {args.colorkey}), "
                f"max color error {err:.0f}")
        if err > 64:
            info += "  ⚠ (far from palette — check colors)"
        print(info, file=sys.stderr)

    return 0


if __name__ == '__main__':
    sys.exit(main())