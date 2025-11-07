#pragma once

#include "../urack_types.h"
#include "../screen_switcher.h"
#include "midi_settings_state.h"
#include "midi_processor.h"

class MidiInfo : public ScreenInterface {
public:
    MidiInfo(Display* display, MidiSettingsState* state, MidiProcessor* processor, ScreenSwitcher* screen_switcher = nullptr);
    void set_screen_switcher(ScreenSwitcher* screen_switcher);

    void enter() override;
    void exit() override;
    void update(Event* event) override;

private:
    MidiSettingsState* state;
    MidiProcessor* processor;
    ScreenSwitcher* screen_switcher;
    void render();
    void handle_input(Event* event);
};
