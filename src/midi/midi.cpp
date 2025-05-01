#include "midi.h"

void MidiRoot::enter() {
    display->clearDisplay();
    display->setTextSize(1);
    display->setTextColor(SSD1306_WHITE);
    display->setCursor(0,0);
    display->println(F("MIDI Mode"));
    display->display();
}

void MidiRoot::exit() {
    display->clearDisplay();
    display->display();
}

void MidiRoot::update(Event* event) {
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

    // Handle events and update display as needed
    display->display();
}

void MidiRoot::handle_note_on(byte channel, byte note, byte velocity) {
    // Handle MIDI note on event
}

void MidiRoot::handle_note_off(byte channel, byte note, byte velocity) {
    // Handle MIDI note off event
}

void MidiRoot::handle_control_change(byte channel, byte number, byte value) {
    // Handle MIDI control change event
} 