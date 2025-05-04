#pragma once

#include "../urack_types.h"
#include "../screen_switcher.h"
#include "midi_settings_state.h"

class MidiSettingsState;

class MidiSettings : public ScreenInterface {
public:
    MidiSettings(Display* display, MidiSettingsState* state);
    void enter() override;
    void exit() override;
    void update(Event* event) override;

private:
    MidiSettingsState* state;
};
