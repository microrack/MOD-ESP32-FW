#pragma once

#include <Arduino.h>
#include <stdint.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcpp"
#include <ESP32Encoder.h>
#pragma GCC diagnostic pop

typedef enum {
    ButtonNone,
    ButtonPress,
    ButtonRelease,
    ButtonHold,
    ButtonSize
} Button;

typedef struct Event {
    int8_t encoder;        // Encoder value
    Button button_a;       // Button A state
    Button button_sw;      // Encoder switch state
    uint32_t button_a_ms;  // Time in ms for Button A
    uint32_t button_sw_ms; // Time in ms for Encoder switch

    static void print(const Event& event);
} Event;

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

    // Debounce variables
    int last_button_a_state;
    int last_button_sw_state;
    unsigned long last_button_a_change_time;
    unsigned long last_button_sw_change_time;
};
