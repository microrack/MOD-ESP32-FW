#include "midi_info.h"

MidiInfo::MidiInfo(Display* display) : ScreenInterface(display) {
    // Initialize any specific properties
}

void MidiInfo::enter() {
    display->clearDisplay();
    display->setTextSize(1);
    display->setTextColor(SSD1306_WHITE);
    display->setCursor(0,0);
    display->println(F("MIDI Info"));
    display->display();
}

void MidiInfo::exit() {
    display->clearDisplay();
    display->display();
}

void MidiInfo::update(Event* event) {
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

    // Update display after handling events
    display->display();
} 