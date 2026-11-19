-- ==========================================
-- 1. СИСТЕМА КАМЕРЫ
-- ==========================================
local _text  = text
local _rect  = rect
local _oval  = oval
local _line  = line
local _pixel = pixel
local _spr   = spr
local _map   = map

cam_x, cam_y = 0, 0

function camera(x, y)
    cam_x = x or 0
    cam_y = y or 0
end

function text(s, x, y, c) _text(s, (x or 0) - cam_x, (y or 0) - cam_y, c) end
function rect(x, y, w, h, c, f) _rect(x - cam_x, y - cam_y, w, h, c, f) end
function oval(x, y, w, h, c, f) _oval(x - cam_x, y - cam_y, w, h, c, f) end
function line(x0, y0, x1, y1, c) _line(x0 - cam_x, y0 - cam_y, x1 - cam_x, y1 - cam_y, c) end
function spr(id, x, y, fx, fy, ck) _spr(id, (x or 0) - cam_x, (y or 0) - cam_y, fx, fy, ck) end

-- pixel: и чтение, и запись — обе с учётом камеры
function pixel(x, y, c)
    if c ~= nil then
        _pixel(x - cam_x, y - cam_y, c)
    else
        return _pixel(x - cam_x, y - cam_y)
    end
end

function map(celx, cely, sx, sy, celw, celh, ck)
    if celx == nil then
        celx = math.floor(cam_x / 8)
        cely = math.floor(cam_y / 8)
        sx = -(cam_x % 8)
        sy = -(cam_y % 8)
        celw = 21
        celh = 16
    end
    _map(celx, cely, sx, sy, celw, celh, ck)
end

-- ==========================================
-- 2. СПРАЙТЫ
-- ==========================================
sprdata(0, [[
11111111
11111111
11111111
11111111
11111111
11111111
11111111
11111111
]])

sprdata(1, [[
44444444
44444444
44444444
44444444
44444444
44444444
44444444
44444444
]])

sprdata(2, [[
00000000
0aaaaaa0
0aaaaaa0
0aaaaaa0
0aeeeaa0
0aeeeaa0
0aaaaaa0
00000000
]])

sprdata(3, [[
00000000
0aaaaaa0
0aeeeaa0
0aeeeaa0
0aeeeaa0
0aeeeaa0
0aaaaaa0
00000000
]])

sprdata(4, [[
000ff000
000ff000
000ff000
000ff000
00ffff00
00ffff00
00f00f00
00f00f00
]])

-- ==========================================
-- 3. КАРТА (30x20 тайлов)
-- ==========================================
mapdata([[
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0
0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0
0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0
0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0
0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0
0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0
0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0
0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0
0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,0
0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0
0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0
0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0
0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0
0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0
0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0
0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0
0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0
0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
]], 30, 20)

-- ==========================================
-- 4. ИНИЦИАЛИЗАЦИЯ
-- ==========================================
function init_game()
    x = 40
    y = 40
    message = "Найди сундук и нажми 'K'!"
end

-- ==========================================
-- 5. ОБНОВЛЕНИЕ
-- ==========================================
function update_game()
    local speed = 2
    local nx, ny = x, y

    if btn(0) then nx = nx - speed end
    if btn(1) then nx = nx + speed end
    if btn(2) then ny = ny - speed end
    if btn(3) then ny = ny + speed end

    local function is_solid(px, py)
        local tx = math.floor(px / 8)
        local ty = math.floor(py / 8)
        local id = tile(tx, ty)       -- было mget
        return id == 0                -- 0 — стена
    end

    if not is_solid(nx, ny) and not is_solid(nx + 7, ny) and
       not is_solid(nx, ny + 7) and not is_solid(nx + 7, ny + 7) then
        x, y = nx, ny
    end

    local center_tx = math.floor((x + 4) / 8)
    local center_ty = math.floor((y + 4) / 8)
    local current_tile = tile(center_tx, center_ty)   -- было mget

    if btnp(5) then
        if current_tile == 2 then
            tile(center_tx, center_ty, 3)             -- было mset
            message = "Сундук открыт!"
        else
            message = "Здесь пусто..."
        end
    end
end

-- ==========================================
-- 6. ОТРИСОВКА
-- ==========================================
function draw_game()
    cls(0)
    camera(x - 76, y - 56)
    map()
    spr(4, x, y, false, false, 0)
    text(message, 5 + cam_x, 10 + cam_y, 15)
end

-- ==========================================
-- 7. ЗАПУСК
-- ==========================================
init_game()

function LUNA()
    update_game()
    draw_game()
end