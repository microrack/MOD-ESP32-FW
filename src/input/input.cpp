#include "input.h"

// Debounce time in milliseconds
#define DEBOUNCE_TIME 50

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
    
    // Initialize debounce variables
    last_button_a_state = HIGH;  // Pullup, so HIGH when not pressed
    last_button_sw_state = HIGH;
    last_button_a_change_time = 0;
    last_button_sw_change_time = 0;
}

Event Input::get_inputs() {
    Event event = {0, ButtonNone, ButtonNone, 0, 0};
    
    unsigned long current_time = millis();

    // Get encoder value
    static int32_t last_encoder = 0;
    int32_t current_encoder = encoder.getCount();
    event.encoder = current_encoder - last_encoder;
    last_encoder = current_encoder;

    // Read button states
    int button_a_reading = digitalRead(BUTTON_A);
    int button_sw_reading = digitalRead(ENCODER_SW);
    
    // Handle Button A with immediate events but debounce inhibition
    if (button_a_reading != last_button_a_state &&  
        (current_time - last_button_a_change_time) > DEBOUNCE_TIME) {
        
        // State changed and debounce period passed since last change
        last_button_a_change_time = current_time;
        last_button_a_state = button_a_reading;
        
        bool button_a_state = (button_a_reading == LOW); // LOW means pressed (pull-up)
        
        if (button_a_state && !button_a_pressed) {
            event.button_a = ButtonPress;
            button_a_pressed = true;
            button_a_press_time = current_time;
        } else if (!button_a_state && button_a_pressed) {
            event.button_a = ButtonRelease;
            event.button_a_ms = current_time - button_a_press_time;
            button_a_pressed = false;
        }
    } else if (button_a_pressed) {
        // Update hold status regardless of debounce
        event.button_a = ButtonHold;
        event.button_a_ms = current_time - button_a_press_time;
    }
    
    // Handle Encoder Switch with immediate events but debounce inhibition
    if (button_sw_reading != last_button_sw_state && 
        (current_time - last_button_sw_change_time) > DEBOUNCE_TIME) {
        
        // State changed and debounce period passed since last change
        last_button_sw_change_time = current_time;
        last_button_sw_state = button_sw_reading;
        
        bool button_sw_state = (button_sw_reading == LOW); // LOW means pressed (pull-up)
        
        if (button_sw_state && !button_sw_pressed) {
            event.button_sw = ButtonPress;
            button_sw_pressed = true;
            button_sw_press_time = current_time;
        } else if (!button_sw_state && button_sw_pressed) {
            event.button_sw = ButtonRelease;
            event.button_sw_ms = current_time - button_sw_press_time;
            button_sw_pressed = false;
        }
    } else if (button_sw_pressed) {
        // Update hold status regardless of debounce
        event.button_sw = ButtonHold;
        event.button_sw_ms = current_time - button_sw_press_time;
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
