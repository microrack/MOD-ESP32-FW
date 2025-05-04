#include "oscilloscope.h"

void OscilloscopeRoot::enter() {
    display->clearDisplay();
    display->setTextSize(1);
    display->setTextColor(SSD1306_WHITE);
    display->setCursor(0,0);
    display->println(F("Oscilloscope Mode"));
    display->display();
}

void OscilloscopeRoot::exit() {
    display->clearDisplay();
    display->display();
}

void OscilloscopeRoot::update(Event* event) {
    if (event == nullptr) return;

    // Handle encoder changes
    if (event->encoder != 0) {
        // Process encoder movement
    }

    // Handle button events
    switch (event->button_a) {
        case ButtonPress:
            // Handle button A press
            break;
        case ButtonRelease:
            // Handle button A release
            break;
        default:
            break;
    }

    switch (event->button_sw) {
        case ButtonPress:
            // Handle encoder switch press
            break;
        case ButtonRelease:
            // Handle encoder switch release
            break;
        default:
            break;
    }

    // Update display after handling events
    display->display();
}