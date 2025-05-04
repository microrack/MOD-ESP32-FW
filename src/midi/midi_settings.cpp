#include "midi_settings.h"

MidiSettings::MidiSettings(Display* display, MidiSettingsState* state)
    : ScreenInterface(display), state(state) {}

void MidiSettings::enter() {
    display->clearDisplay();
    display->setTextSize(1);
    display->setTextColor(SSD1306_WHITE);
    display->setCursor(0,0);
    display->println(F("MIDI Settings"));

    display->display();
}

void MidiSettings::exit() {
    display->clearDisplay();
    display->display();
}

void MidiSettings::update(Event* event) {
    if (event == nullptr) return;

    // Handle encoder for menu navigation
    if (event->encoder != 0) {
        // Move selection up/down
    }

    // Handle button A for selection
    switch (event->button_a) {
        case ButtonPress:
            // Select current item
            break;
        default:
            break;
    }

    // Always update display
    display->display();
}
