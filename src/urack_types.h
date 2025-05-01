#pragma once

#include <stdint.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>

typedef Adafruit_SSD1306 Display;

typedef enum {
    ButtonNone,
    ButtonPress,
    ButtonRelease,
    ButtonSize
} Button;

typedef struct Event {
    int8_t encoder;    // Encoder value
    Button button_a;   // Button A state
    Button button_sw;  // Encoder switch state
    uint32_t button_a_ms; // Time in ms for Button A
    uint32_t button_sw_ms; // Time in ms for Encoder switch

    static void print(const Event& event) {
        if (event.encoder != 0 || event.button_a != ButtonNone || event.button_sw != ButtonNone) {
            Serial.print("Event: ");
            if (event.encoder != 0) {
                Serial.print("Encoder=");
                Serial.print(event.encoder);
                Serial.print(" ");
            }
            if (event.button_a != ButtonNone) {
                Serial.print("ButtonA=");
                Serial.print(event.button_a == ButtonPress ? "Press" : "Release");
                if (event.button_a == ButtonRelease) {
                    Serial.print("(");
                    Serial.print(event.button_a_ms);
                    Serial.print("ms)");
                }
                Serial.print(" ");
            }
            if (event.button_sw != ButtonNone) {
                Serial.print("ButtonSW=");
                Serial.print(event.button_sw == ButtonPress ? "Press" : "Release");
                if (event.button_sw == ButtonRelease) {
                    Serial.print("(");
                    Serial.print(event.button_sw_ms);
                    Serial.print("ms)");
                }
                Serial.print(" ");
            }
            Serial.println();
        }
    }
} Event;

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