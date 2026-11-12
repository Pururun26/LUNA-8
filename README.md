```markdown
# LUNA-8

**A tiny retro game framework for Lua.**

160×120 screen, 8-color palette, 4 sound channels, 512 sprites, 64 save slots.
Native + WebAssembly. One `game.lua` file is the whole cartridge.

---

## Quick start

Create `game.lua`:

```lua
pal(0, 0x2f2f2f)

local x = 80
function luna()
    cls(0)
    rect(x - 2, 58, 5, 5, 7)
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
-- (no raylib calls here — only declarations, dget(), pal(), sprdata())

function luna()
    -- called 60 times per second
end
```

That's it. No `_init`, no `_update`, no `_draw`. One function, 60 FPS.

---

## API

### Graphics

```lua
cls([c=0])                         -- clear screen with color c
pset(x, y, c)                      -- set pixel
pget(x, y) -> c                    -- get pixel (0 if out of bounds)
pal(i, 0xRRGGBB)                   -- set palette color i (0..7)
line(x0, y0, x1, y1, c)
rect(x, y, w, h, c, [filled=true])
oval(x, y, w, h, c, [filled=true]) -- circle when w == h
text(s, [x=0], [y=0], [c=7])       -- 3x4 font, uppercase only
```

Coordinates accept floats — they are floored. `pset(10.7, 5.2, 7)` draws at `(10, 5)`.

### Sprites

```lua
sprdata(id, "02200220\n20022002\n...", [w=8], [h=8])
spr(id, [x=0], [y=0], [flip_x=false], [flip_y=false], [colorkey=-1])
```

`id` — 0..511. `w`, `h` — up to 64. Data is a string of `'0'..'7'` digits (palette indices), whitespace ignored. `colorkey` — color index treated as transparent (`-1` = no transparency).

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

Default palette is a classic 8-color RGB:

| # | Color | Hex | # | Color | Hex |
|---|---|---|---|---|---|
| 0 | black   | `#000000` | 4 | cyan    | `#00FFFF` |
| 1 | red     | `#FF0000` | 5 | magenta | `#FF00FF` |
| 2 | green   | `#00FF00` | 6 | yellow  | `#FFFF00` |
| 3 | blue    | `#0000FF` | 7 | white   | `#FFFFFF` |

Change any color at runtime with `pal(i, 0xRRGGBB)`.

---

## Philosophy

- **Framework, not console.** You write `game.lua`, we run it. No built-in editor, no cartridge format.
- **Minimal API.** 16 functions total. If something can be expressed with existing ones, we don't add it.
- **Sandboxed but not paranoid.** No `io`, `os`, `debug`, `require`, `dofile`, `loadfile`, `load`. Every file goes through the host.
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