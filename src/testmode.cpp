#include "testmode.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "input/input.h"

bool test_mode(Adafruit_SSD1306* display, Input* input) {
    // Clear display
    display->clearDisplay();
    
    // Set text properties
    display->setTextSize(2);
    display->setTextColor(SSD1306_WHITE);
    display->setCursor(0, 0);
    
    // Display "test mode" text
    display->println("test mode");
    display->display();
    
    // Array of flags for checking
    bool flags[TestFlagCount];
    for(size_t i = 0; i < TestFlagCount; i++) {
        flags[i] = false;
    }
    
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

