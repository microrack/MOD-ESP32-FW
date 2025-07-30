#include "midi.h"

MidiRoot::MidiRoot(Display* display)
    : ScreenInterface(display), midi_info(display, &state),
      midi_settings(display, &state), processor(&state) {

    // Initialize MIDI screens array
    midi_screens[0] = &midi_info;
    midi_screens[1] = &midi_settings;

    // Initialize screen switcher with the screens array
    screen_switcher = ScreenSwitcher(midi_screens, 2);
}

void MidiRoot::begin(void) {
    // Initialize all MIDI components
    state.begin();
    processor.begin();
}

void MidiRoot::enter() { screen_switcher.set_screen(0); }

void MidiRoot::exit() { screen_switcher.get_current_screen()->exit(); }

void MidiRoot::update(Event* event) {
    if (event == nullptr)
        return;

    // Handle encoder changes
    if (event->encoder != 0) {
        // Pass to current screen
    }

    // Handle button events - specifically switch screens on button_sw
    // press
    switch (event->button_a) {
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
