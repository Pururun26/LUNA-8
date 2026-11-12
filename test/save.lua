pal(0, 0x2f2f2f)

-- загрузка: dget работает на top-level, т.к. save_open до script_load
local score    = dget(0)
local best     = dget(1)
local counter  = dget(2)
local playtime = dget(3)

function luna()
    cls()

    -- счётчик: dset только при изменении (внутри dset уже есть проверка)
    if btnp(4) then
        score   = score + 1
        counter = counter + 1
        dset(0, score)
        dset(2, counter)
        if score > best then
            best = score
            dset(1, best)
        end
    end

    -- время игры: обновляем не чаще раза в секунду
    if btnp(5) then
        playtime = playtime + 1000
        dset(3, playtime)
    end

    text("SCORE: " .. score,    5,  5, 7)
    text("BEST:  " .. best,     5, 13, 3)
    text("PRESS: " .. counter,  5, 21, 5)
    text("TIME:  " .. playtime, 5, 29, 6)

    text("J = +1 SCORE",   5, 55, 5)
    text("K = +1 SEC",     5, 63, 5)
    text("SAVED TO DISK", 40, 80, 3)
end