```markdown
# LUNA-8

**A tiny retro game framework for Lua.**

160×120 screen, 16-color palette, 4 sound channels, 512 sprites, tilemaps.
Native + WebAssembly. One `game.lua` file is the whole cartridge.

---

## Quick start

Create `game.lua`:

```lua
pal(0, 0x1a0f2e)  -- repaint "black" into dark purple background

local x = 80
function LUNA()
    cls(0)
    rect(x - 2, 58, 5, 5, 15) -- 15 is white
    if btn(0) then x = x - 1 end
    if btn(1) then x = x + 1 end
end
```

Run:
- **Native**: put `game.lua` next to `luna8.exe` and launch, or `luna8.exe my_game.lua`
- **Web**: `emcmake cmake -B build_web && cmake --build build_web`, then put `game.lua` into `build_web/` and serve it

---

## Game structure

```lua
-- top-level code runs once at startup
-- (no raylib calls here — only declarations, dget(), pal(), sprdata(), mapdata())

function LUNA()
    -- called 60 times per second
end
```

That's it. No `_init`, no `_update`, no `_draw`. One function, 60 FPS.

---

## Modularity

Small games fit in one `game.lua`. Larger projects split into multiple files
and get packed into `game.lua` with a single command.

### Project layout

```
mygame/
├── data/
│   ├── main.lua            ← entry point with --[[.lua]] includes
│   ├── player.lua
│   ├── enemies.lua
│   └── bullets.lua
├── tools/
│   └── pack.py             ← ships with LUNA-8
├── game.lua                ← generated (do not edit)
└── luna8.exe
```

### Include syntax

In `data/main.lua`, one include per line:

```lua
--[[player.lua]]
--[[enemies.lua]]
--[[bullets.lua]]

function luna()
    cls(0)
    player_update()
    enemies_update()
    bullets_update()
end
```

The path is **relative to the current file**. Nesting is allowed (up to 32 levels),
cycles are detected and reported with the full chain.

### Pack

From the project root:

```bash
python tools/pack.py data/main.lua
# → ./game.lua
```

`pack.py` replaces each `--[[x.lua]]` line with the contents of `x.lua`,
wraps each module with `-- >>> x.lua` / `-- <<< x.lua` markers
(invisible in Lua, useful in stack traces), and writes the result to `game.lua`
in the **current directory**.

Then run as usual:

```bash
./luna8.exe
```

### Run script

Automate pack + run with a tiny script next to `luna8.exe`.

**Windows — `run.bat`:**

```bat
@echo off
python tools\pack.py data\main.lua
if errorlevel 1 exit /b 1
luna8.exe
```

**Linux / macOS — `run.sh`:**

```bash
#!/bin/sh
set -e
python3 tools/pack.py data/main.lua
./luna8
```

```bash
chmod +x run.sh
./run.sh
```

Double-click `run.bat` (Windows) or `./run.sh` — pack runs, game starts.

---

## API

### Graphics

```lua
cls([c=0])                         -- clear screen with color c
pset(x, y, c)                      -- set pixel
pget(x, y) -> c                    -- get pixel (0 if out of bounds)
pal(i, 0xRRGGBB)                   -- set palette color i (0..15)
line(x0, y0, x1, y1, c)
rect(x, y, w, h, c, [filled=false])
oval(x, y, w, h, c, [filled=false]) -- circle when w == h
text(s, [x=0], [y=0], [c=15])      -- 5x12 UTF-8 font, supports Cyrillic & lowercase
```

Coordinates accept floats — they are floored. `pset(10.7, 5.2, 15)` draws at `(10, 5)`.

#### Font
To move to the next line, simply use the newline character `\n` inside the string. The line height is 14 pixels (12 for the glyph + 2 for spacing).

### Sprites

```lua
sprdata(id, "02200220\n20022002\n...", [w=8], [h=8])
spr(id, [x=0], [y=0], [flip_x=false], [flip_y=false], [colorkey=-1])
```

`id` — 0..511. `w`, `h` — up to 64. Data is a string of **hex digits** `0-9`, `a-f` (or `A-F`), one character per pixel, mapping to palette indices 0..15. Whitespace and any non-hex characters (`\n`, spaces, comments) are ignored. 

`colorkey` — color index (or a table of indices) treated as transparent. 
- Pass `-1` for no transparency. 
- Pass a single integer (e.g., `0`) to make one color transparent.
- Pass a table (e.g., `{0, 1, 2}`) to make multiple colors transparent.

> Index 10 is written `a`, not `10` — one character is exactly one pixel.

### Tilemap

LUNA-8 features a built-in, high-performance tilemap system that runs natively in C. It perfectly supports CSV exports from **Tiled** (and similar editors).

```lua
mapdata(csv_string, [w=20], [h=15])
map([celx=0], [cely=0], [sx=0], [sy=0], [celw=20], [celh=15], [colorkey=-1])
mget(x, y) -> id
mset(x, y, id)
```

- `mapdata(data, [w], [h])` — Loads a map from a CSV string. The string should contain numbers separated by commas (newlines and spaces are ignored). `-1` means an empty tile, `0` is the first sprite, `1` is the second, etc. By default, it expects a 20x15 map (one screen).
- `map(...)` — Draws the map. `celx`, `cely` are the starting tile coordinates. `sx`, `sy` are the screen pixel offsets. `celw`, `celh` define how many tiles to draw (default is 20x15).
- `mget(x, y)` — Returns the tile ID at the given map coordinates (0-based). Returns `-1` if the coordinates are out of bounds or the tile is empty.
- `mset(x, y, id)` — Sets the tile ID at the given map coordinates. Use `-1` to clear a tile.
- `colorkey` — Works exactly like the sprite `colorkey`. Accepts `-1`, an integer, or a table of integers.

**Tiled Integration:**
Export your map layer from Tiled as a **CSV**. The first tile in your tileset will be exported as `0`, which aligns perfectly with `sprdata(0)`. Paste the exported CSV directly into your Lua code.

```lua
-- Example: loading a CSV map directly from Tiled
mapdata([[
0,0,0,0,0,0,0,0,0,0
0,1,1,1,1,1,1,1,1,0
0,1,-1,-1,-1,-1,-1,1,0
0,1,1,1,1,1,1,1,1,0
0,0,0,0,0,0,0,0,0,0
]], 10, 5)
```

### Input

```lua
btn(id, [player=0]) -> bool        -- held down
btnp(id, [player=0]) -> bool       -- pressed this frame (edge)
```

`id`: `0`=left `1`=right `2`=up `3`=down `4`=O `5`=X.
`player 0` — WASD + J/K. `player 1` — arrows + Numpad 4/5.

### Time

```lua
time() -> ms                       -- milliseconds since startup
utime() -> s                       -- unix timestamp
```

### Sound

```lua
tone(freq, [dur=15], [vol=30], [ch=0])
```

`freq` in Hz. `dur` in frames (60 = 1 second, max 60). `vol` 0..100 (`≤25` per channel to avoid clipping). `ch` — 0 (pulse 12.5%), 1 (pulse 25%), 2 (triangle), 3 (noise). `freq=0` or `vol=0` stops the channel.

### Save

```lua
dset(slot, value)                  -- write int64 to slot (0..63)
dget(slot) -> value                -- read int64 from slot
```

Auto-saved on change. Storage: `save/<cart>.bin` next to exe (native) or `localStorage` (web).

---

## Palette

The default palette is 16 colors, sorted ascending by hex value (black → white).
This makes sprite data readable as gradients: `0123456789abcdef` fades smoothly.

| # | Color         | Hex       | # | Color         | Hex       |
|---|---------------|-----------|----|---------------|-----------|
| 0 | black         | `#000000` | 8 | dark red      | `#AA0000` |
| 1 | dark blue     | `#0000AA` | 9 | dark magenta  | `#AA00AA` |
| 2 | dark green    | `#00AA00` | a | brown/orange  | `#AA5500` |
| 3 | dark cyan     | `#00AAAA` | b | light gray    | `#AAAAAA` |
| 4 | dark gray     | `#555555` | c | light red     | `#FF5555` |
| 5 | light blue    | `#5555FF` | d | light magenta | `#FF55FF` |
| 6 | light green   | `#55FF55` | e | yellow        | `#FFFF55` |
| 7 | light cyan    | `#55FFFF` | f | white         | `#FFFFFF` |

Index `0` is black (default background), `15` (`f`) is white (default text color).

Change any color at runtime with `pal(i, 0xRRGGBB)`, `i` in 0..15. Palette swaps are cheap — sprites store only indices, colors are looked up at draw time, so a single `pal()` call re-tints every sprite on screen.

Sprites use one hex character per pixel (`'0'..'9'`, `'a'..'f'`), e.g.:

```lua
sprdata(1, [[
0123456789abcdef
]], 16, 1)
```

---

## Philosophy

- **Framework, not console.** You write `game.lua`, we run it. No built-in editor, no cartridge format.
- **Minimal API.** 21 functions total. If something can be expressed with existing ones, we don't add it.
- **Sandboxed but not paranoid.** No `io`, `os`, `debug`, `require`, `dofile`, `loadfile`, `load`. But `utf8` is fully supported out of the box.
- **Pixel-perfect by design.** 160×120, integer-scaled, letterboxed. No sub-pixel rendering.

---

## Building

### Native (MinGW-w64)

```bash
cmake -G "MinGW Makefiles" -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
build/luna8.exe
```

### WebAssembly

```bash
emcmake cmake -B build_web -DCMAKE_BUILD_TYPE=Release
cmake --build build_web
cp game.lua build_web/
python -m http.server 8000 -d build_web
# open http://localhost:8000
```

---

## Credits

- **Font**: [Monogram](https://datagoblin.itch.io/monogram) by [datagoblin](https://datagoblin.itch.io/), licensed under CC0.