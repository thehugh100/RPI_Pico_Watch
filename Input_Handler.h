#pragma once
#include <iostream>
#include <inttypes.h>
#include <vector>

const uint8_t GPIO_COUNT = 28;

class InputHandler
{
public:
    InputHandler();
    void registerGPIO(uint8_t pin);
    void tick();
    bool isLongPressed(uint8_t pin);
    bool isDown(uint8_t pin, int *downtime = nullptr);
    bool isUp(uint8_t pin, int *uptime = nullptr);
    bool isPressed(uint8_t pin, int *uptime = nullptr);
    bool isReleased(uint8_t pin, int *downtime = nullptr);
    void setDebounceTime(int timeMS);
    void setLongPressTime(int timeMS);
    int getDebounceTime();
    int getLongPressTime();
private:
    uint64_t time_ms();
    bool triggeredLongPress[GPIO_COUNT];
    bool gpioState[GPIO_COUNT];
    bool gpioStatePrev[GPIO_COUNT];
    int gpioLastStateChange[GPIO_COUNT];
    int gpioLastStateChangePrev[GPIO_COUNT];

    int configDebounceTime = 50;
    int configLongPressTime = 500;

    std::vector<uint8_t> trackingGPIO;
};