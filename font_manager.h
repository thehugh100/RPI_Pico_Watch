#include <stdio.h>
#include <cstdio>

class SSH1106;

class FontManager
{
    public:
    void drawCharacterAscii(SSH1106* oled, unsigned char character, uint16_t x, uint16_t y);
    void drawStringAscii(SSH1106* oled, const char* str, size_t len, uint8_t x, uint8_t y);
    void drawCharacterLarge(SSH1106* oled, unsigned char character, uint16_t x, uint16_t y);
    void drawStringLarge(SSH1106* oled, const char* str, size_t len, uint8_t x, uint8_t y);
};