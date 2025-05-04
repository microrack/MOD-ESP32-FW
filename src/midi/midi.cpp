#include "midi.h"
#include <MIDI.h>

// MIDI interface
MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, MIDI);

MidiRoot::MidiRoot(Display* display)
    : ScreenInterface(display),
      midi_info(display, &state),
      midi_settings(display, &state) {

    // Initialize MIDI screens array
    midi_screens[0] = &midi_info;
    midi_screens[1] = &midi_settings;

    // Initialize screen switcher with the screens array
    screen_switcher = ScreenSwitcher(midi_screens, 2);

    // Initialize MIDI
    Serial2.begin(31250, SERIAL_8N1, 16, 17); // MIDI baud rate, Tx=17, Rx=16 for Serial2 on ESP32
    MIDI.begin(MIDI_CHANNEL_OMNI);
}

void MidiRoot::enter() {
    display->clearDisplay();
    display->setTextSize(1);
    display->setTextColor(SSD1306_WHITE);
    display->setCursor(0,0);
    display->println(F("MIDI Mode"));
    display->display();

    // Start with the info screen
    screen_switcher.set_screen(0);
}

void MidiRoot::exit() {
    // Make sure to exit the current sub-screen
    screen_switcher.get_current_screen()->exit();

    display->clearDisplay();
    display->display();
}

void MidiRoot::update(Event* event) {
    if (event == nullptr) return;

    // Handle encoder changes
    if (event->encoder != 0) {
        // Pass to current screen
    }

    // Handle button events - specifically switch screens on button_sw press
    switch (event->button_sw) {
        case ButtonPress:
            // Switch to next MIDI screen
            screen_switcher.set_screen(screen_switcher.get_next());
            break;
        default:
            break;
    }

    // Update the current sub-screen with the event
    screen_switcher.update(event);
}
