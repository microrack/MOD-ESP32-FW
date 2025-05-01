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
    } else if (button_a_state && button_a_pressed) {
        event.button_a = ButtonHold;
        event.button_a_ms = millis() - button_a_press_time;
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
    } else if (button_sw_state && button_sw_pressed) {
        event.button_sw = ButtonHold;
        event.button_sw_ms = millis() - button_sw_press_time;
    } else if (!button_sw_state && button_sw_pressed) {
        event.button_sw = ButtonRelease;
        event.button_sw_ms = millis() - button_sw_press_time;
        button_sw_pressed = false;
    }

    return event;
}

void Event::print(const Event& event) {
    if (event.encoder != 0 || event.button_a != ButtonNone || event.button_sw != ButtonNone) {
        Serial.print("Event: ");
        if (event.encoder != 0) {
            Serial.print("Encoder=");
            Serial.print(event.encoder);
            Serial.print(" ");
        }
        if (event.button_a != ButtonNone) {
            Serial.print("ButtonA=");
            if (event.button_a == ButtonPress) {
                Serial.print("Press");
            } else if (event.button_a == ButtonRelease) {
                Serial.print("Release(");
                Serial.print(event.button_a_ms);
                Serial.print("ms)");
            } else if (event.button_a == ButtonHold) {
                Serial.print("Hold(");
                Serial.print(event.button_a_ms);
                Serial.print("ms)");
            }
            Serial.print(" ");
        }
        if (event.button_sw != ButtonNone) {
            Serial.print("ButtonSW=");
            if (event.button_sw == ButtonPress) {
                Serial.print("Press");
            } else if (event.button_sw == ButtonRelease) {
                Serial.print("Release(");
                Serial.print(event.button_sw_ms);
                Serial.print("ms)");
            } else if (event.button_sw == ButtonHold) {
                Serial.print("Hold(");
                Serial.print(event.button_sw_ms);
                Serial.print("ms)");
            }
            Serial.print(" ");
        }
        Serial.println();
    }
}
