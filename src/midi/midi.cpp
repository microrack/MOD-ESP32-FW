#include "midi.h"

MidiRoot::MidiRoot(Display* display)
    : ScreenInterface(display),
      midi_info(display, &state, &processor, nullptr),
      midi_settings(display, &state, nullptr),
      processor(&state) {

    // Initialize MIDI screens array
    midi_screens[MidiScreen::MidiScreenInfo] = &midi_info;
    midi_screens[MidiScreen::MidiScreenSettings] = &midi_settings;

    // Initialize screen switcher with the screens array
    screen_switcher = ScreenSwitcher(midi_screens, MidiScreen::MidiScreenCount);

    // Set screen_switcher pointer in midi_info and midi_settings
    midi_info.set_screen_switcher(&screen_switcher);
    midi_settings.set_screen_switcher(&screen_switcher);
}

void MidiRoot::begin(void) {
    // Initialize all MIDI components
    state.begin();
    processor.begin();
}

void MidiRoot::enter() {
    screen_switcher.set_screen(MidiScreen::MidiScreenInfo);
}

void MidiRoot::exit() {
    screen_switcher.get_current_screen()->exit();
}

void MidiRoot::update(Event* event) {
    if (event == nullptr) return;

    // Handle encoder changes
    if (event->encoder != 0) {
        // Pass to current screen
    }

    // Update the current sub-screen with the event
    screen_switcher.update(event);
}
