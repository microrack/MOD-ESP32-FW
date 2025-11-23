#include "testmode.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "input/input.h"
#include "board.h"

bool flags[TestFlagCount];
// Flag names
const char* flag_names[TestFlagCount] = {
    "Butt A",
    "Butt SW",
    "Enc CW",
    "Enc CCW",
    "MIDI RX",
    "IN_0",
    "IN_1"
};

// ADC min/max tracking for each channel
int adc0_min = 4095;
int adc0_max = 0;
int adc1_min = 4095;
int adc1_max = 0;


void check_input(Input* input) {
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
}

void check_midi_rx() {
    if (digitalRead(MIDI_RX_PIN) == LOW) {
        flags[TestFlagMidiRx] = true;
    }
}

void check_adc() {
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
    
    // Check IN_0 flag (min < 300 and max > 3800)
    if (adc0_min < 700 && adc0_max > 2400) {
        flags[TestFlagIN_0] = true;
    }
    
    // Check IN_1 flag (min < 300 and max > 3800)
    if (adc1_min < 700 && adc1_max > 2400) {
        flags[TestFlagIN_1] = true;
    }
}

void display_flags(Adafruit_SSD1306* display) {
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

    const int ROW_X = 64;

    display->setCursor(ROW_X, cursor_y);
    
    // Display ADC_0 min/max
    display->println("0: ");
    display->setCursor(ROW_X, display->getCursorY());
    display->println(adc0_min);
    display->setCursor(ROW_X, display->getCursorY());
    display->println(adc0_max);
    display->setCursor(ROW_X, display->getCursorY());
    
    // Display ADC_1 min/max
    display->println("1: ");
    display->setCursor(ROW_X, display->getCursorY());
    display->println(adc1_min);
    display->setCursor(ROW_X, display->getCursorY());
    display->println(adc1_max);
    display->setCursor(ROW_X, display->getCursorY());
    
    display->display();
}

void set_dac(SignalProcessor* signal_processor) {
    static unsigned long last_update = 0;
    static int step = 0;  // 0-5: steps (0=A max, 1=B max, 2=C max, 3=A zero, 4=B zero, 5=C zero)
    
    unsigned long current_time = millis();
    
    // Check if 100ms has passed
    if (current_time - last_update >= 100) {
        last_update = current_time;
        
        // Set the appropriate channel based on step
        switch (step) {
            case 0:  // A = MAX
                signal_processor->out_7bit_value(OutChannelA, 127);
                break;
            case 1:  // B = MAX
                signal_processor->out_7bit_value(OutChannelB, 127);
                break;
            case 2:  // C = MAX
                signal_processor->out_7bit_value(OutChannelC, 127);
                break;
            case 3:  // A = 0
                signal_processor->out_7bit_value(OutChannelA, 0);
                break;
            case 4:  // B = 0
                signal_processor->out_7bit_value(OutChannelB, 0);
                break;
            case 5:  // C = 0
                signal_processor->out_7bit_value(OutChannelC, 0);
                break;
        }
        
        // Move to next step
        step = (step + 1) % 6;
    }
}

bool test_mode(Adafruit_SSD1306* display, Input* input, SignalProcessor* signal_processor) {
    // Configure MIDI_RX_PIN as input
    pinMode(MIDI_RX_PIN, INPUT);

    
    signal_processor->out_7bit_value(OutChannelA, 0);
    signal_processor->out_7bit_value(OutChannelB, 0);
    signal_processor->out_7bit_value(OutChannelC, 127);
    
    // Array of flags for checking
    
    for(size_t i = 0; i < TestFlagCount; i++) {
        flags[i] = false;
    }

    // Enter infinite loop
    for(;;) {
        set_dac(signal_processor);

        check_input(input);
        check_midi_rx();
        check_adc();
        
        display_flags(display);
        
        
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

