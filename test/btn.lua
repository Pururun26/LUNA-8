pal(0, 0x2f2f2f)

-- P1: WASD, квадрат цвет 7
local p1x, p1y = 60, 45
-- P2: стрелки, квадрат цвет 3
local p2x, p2y = 100, 45
-- Счётчики btnp
local s1, s2 = 0, 0

function luna()
    cls(0)

    -- P1 (WASD)
    if btn(0) then p1x = p1x - 1 end
    if btn(1) then p1x = p1x + 1 end
    if btn(2) then p1y = p1y - 1 end
    if btn(3) then p1y = p1y + 1 end

    -- P2 (стрелки)
    if btn(0, 1) then p2x = p2x - 1 end
    if btn(1, 1) then p2x = p2x + 1 end
    if btn(2, 1) then p2y = p2y - 1 end
    if btn(3, 1) then p2y = p2y + 1 end

    -- btnp: J для P1, Numpad 4 для P2
    if btnp(4) then s1 = s1 + 1 end
    if btnp(4, 1) then s2 = s2 + 1 end

    rect(p1x - 2, p1y - 2, 5, 5, 7)
    rect(p2x - 2, p2y - 2, 5, 5, 3)

    text("P1: " .. s1, 5, 5, 7)
    text("P2: " .. s2, 125, 5, 3)
end