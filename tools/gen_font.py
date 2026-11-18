import json
import sys
import argparse

def main():
    ap = argparse.ArgumentParser(description="JSON font -> C header for LUNA-8")
    ap.add_argument("json", nargs="?", default="font.json", help="input JSON")
    ap.add_argument("-o", "--out", default="luna8_font.h", help="output C header")
    args = ap.parse_args()

    try:
        with open(args.json, 'r', encoding='utf-8') as f:
            data = json.load(f)
    except Exception as e:
        print(f"error: cannot read {args.json}: {e}", file=sys.stderr)
        return 1

    glyphs = []
    for char, vals in data.items():
        if len(char) == 1:
            cp = ord(char)
            glyphs.append((cp, vals))

    # Сортируем по коду Unicode для бинарного поиска
    glyphs.sort(key=lambda x: x[0])

    with open(args.out, 'w', encoding='utf-8') as f:
        f.write("/* Auto-generated from font.json — do not edit. */\n")
        f.write("#pragma once\n\n")
        f.write("#include <stdint.h>\n\n")
        
        f.write("#define FONT_CELL_W 5\n")
        f.write("#define FONT_CELL_H 12\n")
        f.write(f"#define FONT_COUNT  {len(glyphs)}\n\n")
        
        f.write("typedef struct {\n")
        f.write("    uint32_t cp;      /* Unicode code point */\n")
        f.write("    uint8_t  rows[12]; /* 12 строк, 5 бит в каждой */\n")
        f.write("    uint8_t  width;   /* Ширина символа (обычно 5) */\n")
        f.write("} L8FontGlyph;\n\n")
        
        f.write(f"static const L8FontGlyph luna8_font[{len(glyphs)}] = {{\n")
        for cp, rows in glyphs:
            # Форматируем строки как 0x1F для красоты
            rows_str = ", ".join(f"0x{r:02X}" for r in rows)
            # Красивое представление символа для комментария
            if cp == 32: char_repr = "space"
            elif cp == 92: char_repr = "\\"
            elif cp == 39: char_repr = "'"
            elif 32 <= cp <= 126: char_repr = chr(cp)
            else: char_repr = "U+{:04X}".format(cp)
            
            f.write(f"    {{0x{cp:04X}, {{{rows_str}}}, 5}}, /* '{char_repr}' */\n")
        f.write("};\n\n")
        
        # Добавляем быструю функцию поиска прямо в хедер
        f.write("/* Бинарный поиск глифа по Unicode code point */\n")
        f.write("static inline const L8FontGlyph* luna8_get_glyph(uint32_t cp) {\n")
        f.write("    int l = 0, r = FONT_COUNT - 1;\n")
        f.write("    while (l <= r) {\n")
        f.write("        int m = l + (r - l) / 2;\n")
        f.write("        if (luna8_font[m].cp == cp) return &luna8_font[m];\n")
        f.write("        if (luna8_font[m].cp < cp) l = m + 1;\n")
        f.write("        else r = m - 1;\n")
        f.write("    }\n")
        f.write("    return NULL;\n")
        f.write("}\n")

    print(f"Generated {args.out} with {len(glyphs)} glyphs")
    return 0

if __name__ == '__main__':
    sys.exit(main())