#include "midi.h"
#include "midi_info.h"
#include "util.h"

MidiInfo::MidiInfo(Display* display, MidiSettingsState* state, MidiProcessor* processor, ScreenSwitcher* screen_switcher)
    : ScreenInterface(display), state(state), processor(processor), screen_switcher(screen_switcher) {
    // Initialize any specific properties
}

void MidiInfo::set_screen_switcher(ScreenSwitcher* screen_switcher) {
    this->screen_switcher = screen_switcher;
}

void MidiInfo::enter() {
    
}

void MidiInfo::exit() {

}

void MidiInfo::render() {
    display->clearDisplay();
    display->setTextSize(1);
    display->setTextColor(SSD1306_WHITE);
    display->setCursor(0,0);

    char buffer[32];
    display->setTextSize(2);
    sprintf(buffer, "BPM: %s", state->get_bpm_str());
    display->println(buffer);
    display->setTextSize(1);

    sprintf(buffer, "Ch: %s  Clk: %s",
            state->get_midi_channel_str(),
            state->get_midi_clk_type_str());
    display->println(buffer);
    display->println();

    sprintf(buffer, "A: %s %d", state->get_midi_out_type_str(0), processor->last_out[0]);
    display->println(buffer);
    sprintf(buffer, "B: %s %d", state->get_midi_out_type_str(1), processor->last_out[1]);
    display->println(buffer);
    sprintf(buffer, "C: %s %d", state->get_midi_out_type_str(2), processor->last_out[2]);
    display->println(buffer);

    display->display();
}

void MidiInfo::handle_input(Event* event) {
    if (event == nullptr) return;

    if (event->encoder != 0) {
        state->set_bpm(clampi(state->get_bpm() + event->encoder,
                           state->get_min_bpm(),
                           state->get_max_bpm()));
        state->store(); // TODO: delay before storing for saving FLASH
    }

    if(event->button_sw == ButtonPress) {
        screen_switcher->set_screen(MidiScreen::MidiScreenSettings);
    }
}

void MidiInfo::update(Event* event) {
    handle_input(event);
    render();
}
