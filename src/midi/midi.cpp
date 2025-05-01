#include "midi.h"
#include <MIDI.h>

// MIDI interface
MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, MIDI);

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

// Constructor - Corrected definition
MidiRoot::MidiRoot(Display* display) : ScreenInterface(display) {
    Serial2.begin(31250, SERIAL_8N1, 16, 17); // MIDI baud rate, Tx=17, Rx=16 for Serial2 on ESP32

    // Initialize MIDI
    MIDI.begin(MIDI_CHANNEL_OMNI);
    // Add handlers if needed (e.g., MIDI.setHandleNoteOn(...))
    // MIDI.setHandleNoteOn(MidiRoot::handle_note_on); // Example: Need to define this static handler
}
