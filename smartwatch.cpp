#include <stdio.h>
#include "pico/stdlib.h"
#include <cstdio>
#include <math.h>

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <map>
#include <math.h>
#include "SSH1106_SPI_Lite.h"
#include "pico/util/datetime.h"

#include "font_manager.h"
#include "fonts/moonphase.h"
#include "fonts/moonphase_names.h"

extern "C" {
    #include "hardware/rtc.h"
}

const char* months[] = {
    "Jan",
    "Feb",
    "Mar",
    "Apr",
    "May",
    "Jun",
    "Jul",
    "Aug",
    "Sep",
    "Oct",
    "Nov",
    "Dec"
};
const char* weekdays[] = {
    "Sun",
    "Mon",
    "Tue",
    "Wed",
    "Thu",
    "Fri",
    "Sat"
};

const char* moonphases[] = {
    "New Moon",
    "Waxing Crescent",
    "First Quarter",
    "Waxing Gibbous",
    "Full Moon",
    "Waning Gibbous",
    "Third Quarter",
    "Waning Crescent"
};


uint64_t lastDisplay = 0;
uint64_t lastMillis = 0;
uint64_t second = 0;
uint64_t lastSecond = 0;

#define SCREEN_TIME 0
#define SCREEN_MOON 1
int screen = SCREEN_TIME;

uint64_t time_ms()
{
    return us_to_ms(time_us_64());
}

float moonAge = 8;
float moonAgeSetMS = 0;
float moonPhaseSmooth = 0;

void drawMoonPhase(SSH1106 *oled, FontManager *fontManager, datetime_t* dt)
{
    //int currentPhase = (time_ms() / 32) % 59;

    float daysSinceSet = (time_ms() - moonAgeSetMS) / 86400000.f;
    float targetMoonPhase = moonAge + daysSinceSet;
    moonPhaseSmooth += (targetMoonPhase - moonPhaseSmooth) * .08;


    int currentPhase = (int)(fmod(moonPhaseSmooth, 29.53f) * 8.) % 236;
    int currentFrame = currentPhase * 64;

    for(int y = 0; y < 64; ++y)
    {
        for(int x = 0; x < 64; ++x)
        {
            int lookup = (y < 32 ? y : 63-y) * 15104 + x + currentFrame;
            oled->drawPixel(x, y, (moonphase[lookup / 8] >> (lookup % 8)) & 1);
        }
    }

    char ageText[10] = {0};
    int ageTextLen = sprintf(ageText, "%02d:%02d\nDays:\n%0.2f", dt->hour, dt->min, fmod(targetMoonPhase, 29.53f));
    fontManager->drawStringAscii(oled, ageText, ageTextLen, 68, 63);

    int moonphaseIDX = fmod(moonPhaseSmooth, 29.53f) / 3.69125;

    for(int y = 0; y < 16; ++y)
    {
        for(int x = 0; x < 48; ++x)
        {
            oled->drawPixel(70 + x, 48 + y, moonphase_names[(y + moonphaseIDX * 16) * 48 + x]);
        }
    }
}

void drawTimeScreen(SSH1106 *oled, FontManager *fontManager, datetime_t* dt, uint64_t lastMillis)
{
    char timeD[8] = {0};
    sprintf(timeD, "%02d:%02d", dt->hour, dt->min);

    char sec[16] = {0};
    int32_t ms = (time_ms() - lastMillis) / 100;
    int secLen = sprintf(sec, "%02d.%01d", dt->sec, ms);

    fontManager->drawStringLarge(oled, timeD, 5, 0, 16);
    char date1[16] = {0};
    int date1Len = sprintf(date1, "%s %02d %s %02d", weekdays[dt->dotw], dt->day, months[dt->month-1], dt->year - 2000);
    fontManager->drawStringAscii(oled, date1, date1Len, 0, 63);
    fontManager->drawStringAscii(oled, sec, 4, 44, 48); 
}

void updateDisplay(SSH1106 *oled, FontManager *fontManager, datetime_t* dt, uint64_t lastMillis)
{
    oled->clear();

    switch (screen)
    {
    case SCREEN_TIME:
        drawTimeScreen(oled, fontManager, dt, lastMillis);
        break;
    case SCREEN_MOON:
        drawMoonPhase(oled, fontManager, dt);
        break;
    }

    oled->display();
}

uint16_t receiveLen = 0;
char receiveBuffer[256] = {0};

void clearReceiveBuf()
{
    receiveLen = 0;
    for(int i = 0; i < 256; ++i)
    {
        receiveBuffer[i] = 0;
    }
}

void parseCommand(std::string command, SSH1106 *oled)
{
    std::map<std::string, std::function<void(std::vector<std::string>&)>> commands;

    commands["uptime"] = [&](std::vector<std::string>& tokens) {
        if(tokens.size() == 1)
        {
            std::cout << "Uptime: " << time_ms() << " ms" << std::endl;
        }
        else
        {
            std::cout << "Too many arguments" << std::endl;
        }
    };
    commands["moonphase"] = [&](std::vector<std::string>& tokens) {
        if(tokens.size() == 2)
        {
            moonAge = std::atof(tokens[1].c_str());
            moonAgeSetMS = time_ms();
        }
        else
        {
            std::cout << "Too many arguments" << std::endl;
        }
    };
    commands["screen"] = [&](std::vector<std::string>& tokens) {
        if(tokens.size() == 2)
        {
            screen = std::atoi(tokens[1].c_str());
        }
        else
        {
            std::cout << "Too many arguments" << std::endl;
        }
    };
    commands["reset_oled"] = [&](std::vector<std::string>& tokens) {
        if(tokens.size() == 1)
        {
            oled->clear();
            oled->init();
        }
        else
        {
            std::cout << "Too many arguments" << std::endl;
        }
    };
    commands["time"] = [&](std::vector<std::string>& tokens) {
        if(tokens.size() == 8)
        {
            // second, minute, hour, day, dotw, month, year
            datetime_t dt = {0};
            rtc_get_datetime(&dt);
            dt.sec = std::atoi(tokens[1].c_str());
            dt.min = std::atoi(tokens[2].c_str());
            dt.hour = std::atoi(tokens[3].c_str());
            dt.day = std::atoi(tokens[4].c_str());
            dt.dotw = std::atoi(tokens[5].c_str());
            dt.month = std::atoi(tokens[6].c_str());
            dt.year = std::atoi(tokens[7].c_str());

            lastMillis = time_ms();

            rtc_set_datetime(&dt);
        }
        else
        {
            std::cout << "Incorrect Arguments, expecting: second minute hour day dotw month year" << std::endl;
        }
    };
    commands["help"] = [&](std::vector<std::string>& tokens) {
        std::cout << "Available Commands: " << std::endl;
        for(auto &i : commands)
        {
            std::cout << i.first << std::endl;
        }
    };

    std::cout << "Command is: " << command << std::endl;
    std::vector<std::string> tokens;
    std::string currentToken = "";
    for(auto &i : command)
    {
        if(i != ' ')
        {
            currentToken += i;
        }
        else
        {
            tokens.push_back(currentToken);
            currentToken = "";
        }
    }
    if(currentToken != "")
        tokens.push_back(currentToken);

    if(tokens.size() >= 1)
    {
        if (commands.find(tokens[0]) != commands.end())
        {
            commands[tokens[0]](tokens);
        }
        else
        {
            std::cout << "Command not found." << std::endl;
        }
    }
    else
    {
        std::cout << "Nothing to parse." << std::endl;
    }
}

void receiveSerial(SSH1106 *oled)
{    
    while(1)
    {
        int rec = getchar_timeout_us(0);
        if(rec != PICO_ERROR_TIMEOUT)
        {
            putchar(rec);

            if(receiveLen >= 255)
            {
                printf("Receive Buffer Full\r\n");
                clearReceiveBuf();
                break;
            }

            if(rec == '\n' || rec == '\r')
            {
                parseCommand(std::string(receiveBuffer, receiveLen), oled);
                clearReceiveBuf();
            }
            else
            {
                receiveBuffer[receiveLen] = rec;
                receiveLen++;
            }
        }
        else
        {
            break;
        }
    }
}

int main() 
{
    stdio_init_all();
    printf("Booted");

    SSH1106 oled;
    FontManager fontManager;

    oled.init();

    datetime_t dt = {
        .year  = 2021,
        .month = 9,
        .day   = 2,
        .dotw  = 4,
        .hour  = 21,
        .min   = 57,
        .sec   = 0
    };

    rtc_init();
    rtc_set_datetime(&dt);

    while(1)
    {
        datetime_t dt = {0};
        rtc_get_datetime(&dt);

        lastSecond = second;
        second = dt.sec;

        if(second != lastSecond)
        {
            lastMillis = time_ms();
        }

        if(time_ms() - lastDisplay > 16)
        {
            lastDisplay = time_ms();
            updateDisplay(&oled, &fontManager, &dt, lastMillis);
            receiveSerial(&oled);
        }
    }
}
