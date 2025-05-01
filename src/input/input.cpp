#include "input.h"

Input::Input() {
    // Initialize encoder
    encoder.attachHalfQuad(ENCODER_A, ENCODER_B);
    encoder.setCount(0);

    // Configure pins
    pinMode(BUTTON_A, INPUT_PULLUP);
    pinMode(ENCODER_SW, INPUT_PULLUP);
    pinMode(ENCODER_A, INPUT_PULLUP);
    pinMode(ENCODER_B, INPUT_PULLUP);

    button_a_pressed = false;
    button_sw_pressed = false;
    button_a_press_time = 0;
    button_sw_press_time = 0;
}

Event Input::get_inputs() {
    Event event = {0, ButtonNone, ButtonNone, 0, 0};

    // Get encoder value
    static int32_t last_encoder = 0;
    int32_t current_encoder = encoder.getCount();
    event.encoder = current_encoder - last_encoder;
    last_encoder = current_encoder;

    // Handle Button A
    bool button_a_state = !digitalRead(BUTTON_A);
    if (button_a_state && !button_a_pressed) {
        event.button_a = ButtonPress;
        button_a_pressed = true;
        button_a_press_time = millis();
    } else if (!button_a_state && button_a_pressed) {
        event.button_a = ButtonRelease;
        event.button_a_ms = millis() - button_a_press_time;
        button_a_pressed = false;
    }

    // Handle Encoder Switch
    bool button_sw_state = !digitalRead(ENCODER_SW);
    if (button_sw_state && !button_sw_pressed) {
        event.button_sw = ButtonPress;
        button_sw_pressed = true;
        button_sw_press_time = millis();
    } else if (!button_sw_state && button_sw_pressed) {
        event.button_sw = ButtonRelease;
        event.button_sw_ms = millis() - button_sw_press_time;
        button_sw_pressed = false;
    }

    return event;
}
