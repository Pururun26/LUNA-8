pal(0, 0x2f2f2f)

function luna()
    cls(0)
    text("HELLO LUNA-8", 10, 10, 7)
    text("abcdefghij", 10, 20, 3)              -- апперкейс автоматом
    text("score: 1234", 10, 30, 6)
    text("line1\nline2\nline3", 10, 45, 5)     -- многострочный
    text("width test: iiii WWWW", 10, 75, 2)   -- i уже W уже
end