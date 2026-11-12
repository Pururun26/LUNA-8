pal(0, 0x2f2f2f)

local notes = {262, 330, 392, 523}   -- C E G C
local idx = 1

function luna()
    cls(0)
    text("SPACE = BEEP", 30, 30, 7)
    text("CH 1 = SQUARE", 30, 40, 5)
    text("CH 2 = PULSE",  30, 47, 5)
    text("CH 3 = TRIANGLE", 30, 54, 5)
    text("CH 4 = NOISE", 30, 61, 5)

    if btnp(4) then                    -- J
        tone(1000, 10, 40, 0)
    end
    if btnp(5) then                    -- K
        tone(0, 0, 0, 0)               -- стоп канала 0
        tone(0, 0, 0, 2)
    end
end