#include "Input_Handler.h"
#include "pico/stdlib.h"

uint64_t InputHandler::time_ms()
{
    return us_to_ms(time_us_64());
}

InputHandler::InputHandler()
{
    for(int i = 0; i < 28; ++i)
    {
        gpioState[i] = gpioStatePrev[i] = gpioLastStateChange[i] = 0;
    }
}

void InputHandler::setDebounceTime(int timeMS)
{
    configDebounceTime = timeMS;
}

void InputHandler::setLongPressTime(int timeMS)
{
    configLongPressTime = timeMS;
}

int InputHandler::getDebounceTime()
{
    return configDebounceTime;
}

int InputHandler::getLongPressTime()
{
    return configLongPressTime;
}

void InputHandler::registerGPIO(uint8_t pin)
{
    trackingGPIO.push_back(pin);
}

void InputHandler::tick()
{
    for(int i = 0; i < trackingGPIO.size(); ++i)
    {
        uint8_t pin = trackingGPIO[i];
        gpioStatePrev[pin] = gpioState[pin];

        bool val = !gpio_get(pin); /* Pullup */

        if(val != gpioStatePrev[pin])
        {
            if(time_ms() - gpioLastStateChange[pin] < 50) //50ms debouncing
            {
                continue;
            }
            gpioLastStateChangePrev[pin] = gpioLastStateChange[pin];
            gpioLastStateChange[pin] = time_ms();
        }
        gpioState[pin] = val;

        if(gpioState[pin] == 0 && gpioStatePrev[pin] == 1) // button released
        {
            //when button is released reset the long press action
            triggeredLongPress[pin] = 0;
        }
    }
}

bool InputHandler::isLongPressed(uint8_t pin)
{
    if((time_ms() - gpioLastStateChange[pin] > 500) && !triggeredLongPress[pin] && gpioState[pin])
    {
        triggeredLongPress[pin] = 1;
        return true;
    }

    return false;
}

bool InputHandler::isDown(uint8_t pin, int *downtime)
{
    if(downtime != nullptr)
        *downtime = time_ms() - gpioLastStateChange[pin];

    return gpioState[pin] == 1;
}

bool InputHandler::isUp(uint8_t pin, int *uptime)
{
    if(uptime != nullptr)
        *uptime = time_ms() - gpioLastStateChange[pin];

    return gpioState[pin] == 0;
}

bool InputHandler::isPressed(uint8_t pin, int *uptime)
{
    if(uptime != nullptr)
        *uptime = time_ms() - gpioLastStateChangePrev[pin];

    return gpioState[pin] == 1 && gpioStatePrev[pin] == 0;
}

bool InputHandler::isReleased(uint8_t pin, int *downtime)
{
    if(downtime != nullptr)
        *downtime = time_ms() - gpioLastStateChangePrev[pin];

    return gpioState[pin] == 0 && gpioStatePrev[pin] == 1;
}