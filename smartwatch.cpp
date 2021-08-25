#include <stdio.h>
#include "pico/stdlib.h"
#include <cstdio>
#include <math.h>

#include <iostream>
#include <string>
#include <vector>

#include "SSH1106_SPI_Lite.h"
#include "pico/util/datetime.h"

#include "font_manager.h"

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

uint64_t time_ms()
{
    return us_to_ms(time_us_64());
}

void updateDisplay(SSH1106 *oled, FontManager *fontManager, datetime_t* dt, uint64_t lastMillis)
{
    char timeD[8] = {0};
    sprintf(timeD, "%02d:%02d", dt->hour, dt->min);

    char sec[16] = {0};
    int32_t ms = (time_ms() - lastMillis) / 100;
    int secLen = sprintf(sec, "%02d.%01d", dt->sec, ms);
    
    oled->clear();
    fontManager->drawStringLarge(oled, timeD, 5, 0, 16);

    char date1[16] = {0};
    int date1Len = sprintf(date1, "%s %02d %s %02d", weekdays[dt->dotw], dt->day, months[dt->month-1], dt->year - 2000);
    fontManager->drawStringAscii(oled, date1, date1Len, 0, 63);

    fontManager->drawStringAscii(oled, sec, 4, 44, 48);

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

void parseCommand(std::string command)
{
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

    /* for(auto &i : tokens)
    {
        printf("Token: %s\r\n", i.c_str());
    } */
    
    if(tokens.size() >= 1)
    {
        if(tokens[0] == "time")
        {
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
                rtc_set_datetime(&dt);
            }
            else
            {
                std::cout << "Incorrect Arguments, expecting: second minute hour day dotw month year" << std::endl;
            }
        }
    }
    else
    {
        std::cout << "Nothing to parse." << std::endl;
    }
}

void receiveSerial()
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
                parseCommand(std::string(receiveBuffer, receiveLen));
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
        .month = 8,
        .day   = 22,
        .dotw  = 0,
        .hour  = 5,
        .min   = 1,
        .sec   = 0
    };

    rtc_init();
    rtc_set_datetime(&dt);

    uint64_t lastDisplay = 0;

    uint64_t lastMillis = 0;
    uint64_t second = 0;
    uint64_t lastSecond = 0;

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
            receiveSerial();
        }
    }
}
