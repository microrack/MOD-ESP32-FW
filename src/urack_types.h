#pragma once

#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <stdint.h>
#include "input/input.h"

typedef Adafruit_SSD1306 Display;

class ScreenInterface {
public:
    ScreenInterface(Display* display) : display(display) {}
    virtual ~ScreenInterface() = default;

    // Called when the screen is entered
    virtual void enter() = 0;

    // Called when the screen is exited
    virtual void exit() = 0;

    // Called to update the screen state based on events
    virtual void update(Event* event) = 0;

protected:
    Display* display;
};
