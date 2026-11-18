-- Полный текст, который мы будем печатать
local full_text = "Привет, LUNA-8! Это тест\nUTF-8. Теперь текст\nможно безопасно резать\nпо буквам, не ломая\nкириллицу."

-- Считаем реальное количество символов один раз при загрузке
local total_chars = utf8.len(full_text)

-- Переменные для эффекта печатной машинки
local char_idx = 0 -- Сколько символов уже напечатано (начинаем с 0)
local timer = 0
local speed = 2 -- Каждые 2 кадра добавляем по букве (30 символов в секунду)

function LUNA()
    cls(0) -- Очищаем экран (черный фон)

    -- 1. ЛОГИКА ПЕЧАТНОЙ МАШИНКИ
    -- Печатаем только если еще не дошли до конца
    if char_idx < total_chars then
        timer = timer + 1
        if timer >= speed then
            char_idx = char_idx + 1
            timer = 0
        end
    end

    -- 2. БЕЗОПАСНАЯ ОБРЕЗКА СТРОКИ
    -- utf8.offset находит номер байта, с которого начинается СЛЕДУЮЩИЙ символ.
    -- Мы берем байты до него (поэтому -1), чтобы показать уже напечатанную часть.
    local next_char_byte = utf8.offset(full_text, char_idx + 1)
    local current_text = ""
    
    if next_char_byte then
        current_text = string.sub(full_text, 1, next_char_byte - 1)
    else
        current_text = full_text -- Дошли до самого конца
    end

    -- 3. ОТРИСОВКА
    -- Индекс 3 - белый, индекс 11 - светло-красный, индекс 9 - светло-циан
    text("--- ТЕСТ UTF-8 ---", 10, 10, 11)
    
    -- Рисуем печатающийся текст
    text(current_text, 10, 30, 3)

    -- 4. ТЕСТ utf8.char()
    local arrow_right = utf8.char(0x2192) -- Стрелка вправо (→)
    local arrow_left = utf8.char(0x2190)  -- Стрелка влево (←)
    
    text("Символы: " .. arrow_left .. " " .. arrow_right, 10, 100, 9)

    -- 5. ОТЛАДОЧНАЯ ИНФОРМАЦИЯ
    local debug_info = "Символов: " .. char_idx .. " / " .. total_chars
    text(debug_info, 10, 110, 2) -- Серый цвет (индекс 2)

    -- 6. СБРОС ПО КНОПКЕ X (кнопка 5)
    if btnp(5) then
        char_idx = 0
        timer = 0
    end
end