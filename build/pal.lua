function LUNA()
    cls()

    -- Серый фон под всей сеткой, иначе чёрный свотч (индекс 0) не виден
    rect(0, 0, 74, 74, 1, true)

    -- Сетка 4x4: индексы 0..15, каждый свотч 16x16 с 1px зазором
    for i = 0, 15 do
        local col = i % 4
        local row = math.floor(i / 4)
        rect(col * 18 + 1, row * 18 + 1, 16, 16, i, true)
    end
end