#include "font_manager.h"
#include "SSH1106_SPI_Lite.h"
#include "fonts/numbers_26x32.h"
#include "fonts/ascii_10x16.h"

void FontManager::drawCharacterAscii(SSH1106* oled, unsigned char character, uint16_t x, uint16_t y)
    {
        // 10x16 character
        const uint16_t charWidth = 10;
        const uint16_t charHeight = 16;
        //character indexes at ' ' (32)
        uint16_t characterX = (character-' ') * charWidth;

        for(uint16_t dy = 0; dy < charHeight; ++dy)
        {
            for(uint16_t dx = 0; dx < charWidth; ++dx)
            {
                uint8_t val = ascii_10x16[dy * 950 + characterX + dx];
                if(val)
                    oled->drawPixel((dx + x) % 128, (dy + y) % 64, val);
            }
        }
    }

    void FontManager::drawStringAscii(SSH1106* oled, const char* str, size_t len, uint8_t x, uint8_t y)
    {
        uint8_t ox = x;
        for(int i = 0; i < len; ++i)
        {
            if(str[i] == '\n')
            {
                x = ox;
                y += 16;
            }
            else
            {
                drawCharacterAscii(oled, str[i], x, y);
                x+=10;
            }
        }
    }

    void FontManager::drawCharacterLarge(SSH1106* oled, unsigned char character, uint16_t x, uint16_t y)
    {
        //character indexes at '0' (48)
        uint16_t characterX = (character-48) * 26;

        // 26x32 character
        for(uint16_t dy = 0; dy < 32; ++dy)
        {
            for(uint16_t dx = 0; dx < 26; ++dx)
            {
                oled->drawPixel(dx + x, dy + y, numbers_26x32[dy * 286 + characterX + dx]);
            }
        }
    }

    void FontManager::drawStringLarge(SSH1106* oled, const char* str, size_t len, uint8_t x, uint8_t y)
    {
        for(int i = 0; i < len; ++i)
        {
            drawCharacterLarge(oled, str[i], x, y);
            x+=26;
        }
    }