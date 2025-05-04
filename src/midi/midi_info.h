#pragma once

#include "../urack_types.h"
#include "midi_settings_state.h"

class MidiInfo : public ScreenInterface {
public:
    MidiInfo(Display* display, MidiSettingsState* state);

    void enter() override;
    void exit() override;
    void update(Event* event) override;

private:
    MidiSettingsState* state;

    void render();
    void handle_input(Event* event);
};
