#include "testmode.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "input/input.h"
#include "board.h"

bool test_mode(Adafruit_SSD1306* display, Input* input) {
    // Configure MIDI_RX_PIN as input
    pinMode(MIDI_RX_PIN, INPUT);
    
    // Array of flags for checking
    bool flags[TestFlagCount];
    for(size_t i = 0; i < TestFlagCount; i++) {
        flags[i] = false;
    }
    
    // Flag names
    const char* flag_names[TestFlagCount] = {
        "Butt A",
        "Butt SW",
        "Enc CW",
        "Enc CCW",
        "MIDI RX"
    };
    
    // ADC min/max tracking for each channel
    int adc0_min = 4095;
    int adc0_max = 0;
    int adc1_min = 4095;
    int adc1_max = 0;
    
    // Enter infinite loop
    for(;;) {
        Event event = input->get_inputs();
        
        // Check button_a press
        if (event.button_a == ButtonPress) {
            flags[TestFlagButtonA] = true;
        }
        
        // Check button_sw press
        if (event.button_sw == ButtonPress) {
            flags[TestFlagButtonSw] = true;
        }

        if (event.encoder > 0) {
            flags[TestFlagEncoderCW] = true; // Clockwise
        } else if (event.encoder < 0) {
            flags[TestFlagEncoderCCW] = true; // Counterclockwise
        }
        
        // Check MIDI_RX_PIN (LOW = flag set)
        if (digitalRead(MIDI_RX_PIN) == LOW) {
            flags[TestFlagMidiRx] = true;
        }
        
        // Read ADC values
        int adc0_val = analogRead(ADC_0);
        int adc1_val = analogRead(ADC_1);
        
        // Update min/max for ADC_0
        if (adc0_val < adc0_min) {
            adc0_min = adc0_val;
        }
        if (adc0_val > adc0_max) {
            adc0_max = adc0_val;
        }
        
        // Update min/max for ADC_1
        if (adc1_val < adc1_min) {
            adc1_min = adc1_val;
        }
        if (adc1_val > adc1_max) {
            adc1_max = adc1_val;
        }
        
        // Update display
        display->clearDisplay();
        display->setTextSize(1);
        display->setTextColor(SSD1306_WHITE);
        display->setCursor(0, 0);
        
        // Display "test mode" header
        display->setTextSize(2);
        display->println("test mode");
        display->setTextSize(1);

        int16_t cursor_y = display->getCursorY();
        
        // Display all flags with their status
        for (int i = 0; i < TestFlagCount; i++) {
            display->print(flag_names[i]);
            if (flags[i]) {
                display->println(" OK");
            } else {
                display->println("");
            }
        }

        const int ROW_X = 50;

        display->setCursor(ROW_X, cursor_y);
        
        // Display ADC_0 min/max
        display->print("0 min: ");
        display->println(adc0_min);
        display->setCursor(ROW_X, display->getCursorY());
        display->print("0 max: ");
        display->println(adc0_max);
        display->setCursor(ROW_X, display->getCursorY());
        
        // Display ADC_1 min/max
        display->print("1 min: ");
        display->println(adc1_min);
        display->setCursor(ROW_X, display->getCursorY());
        display->print("1 max: ");
        display->println(adc1_max);
        display->setCursor(ROW_X, display->getCursorY());
        
        display->display();
        
        // Check if all flags are set
        bool all_flags_set = true;
        for (int i = 0; i < TestFlagCount; i++) {
            if (!flags[i]) {
                all_flags_set = false;
                break;
            }
        }
        
        if (all_flags_set) {
            return true;
        }
    }
}

