#pragma once

#include "../urack_types.h"
#include <ESP32Encoder.h>

class Input {
public:
    Input();
    Event get_inputs();
    void begin();

private:
    ESP32Encoder encoder;
    static const int BUTTON_A = 38;
    static const int ENCODER_SW = 39;
    static const int ENCODER_A = 34;
    static const int ENCODER_B = 35;

    uint32_t button_a_press_time;
    uint32_t button_sw_press_time;
    bool button_a_pressed;
    bool button_sw_pressed;
};
