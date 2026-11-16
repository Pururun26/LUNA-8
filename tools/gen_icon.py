#!/usr/bin/env python3
"""gen_icon.py — PNG → src/luna8_icon.h (RGBA, 32×32 по умолчанию).

Usage:
    python tools/gen_icon.py icon.png src/luna8_icon.h [size]
"""

import sys
from PIL import Image

def main():
    if len(sys.argv) < 3:
        print(f"usage: {sys.argv[0]} <icon.png> <out.h> [size=32]", file=sys.stderr)
        return 1

    src, dst = sys.argv[1], sys.argv[2]
    size = int(sys.argv[3]) if len(sys.argv) > 3 else 32

    img = Image.open(src).convert('RGBA')
    img = img.resize((size, size), Image.NEAREST)
    px = img.load()

    w, h = img.size
    with open(dst, 'w', encoding='utf-8') as f:
        f.write(f'/* Auto-generated from {src} — do not edit. */\n')
        f.write('#pragma once\n#include <stdint.h>\n\n')
        f.write(f'#define ICON_W {w}\n')
        f.write(f'#define ICON_H {h}\n\n')
        f.write(f'static const uint8_t luna8_icon_rgba[{w} * {h} * 4] = {{\n')
        for y in range(h):
            f.write('    ')
            for x in range(w):
                r, g, b, a = px[x, y]
                f.write(f'{r},{g},{b},{a},')
            f.write('\n')
        f.write('};\n')

    print(f"OK: {dst}  ({w}×{h}, {len(open(dst).read())} bytes)")
    return 0

if __name__ == '__main__':
    sys.exit(main())