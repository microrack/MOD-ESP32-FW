#pragma once

#include "../urack_types.h"
#include "midi_settings_state.h"
#include "midi_processor.h"

class MidiInfo : public ScreenInterface {
public:
    MidiInfo(Display* display, MidiSettingsState* state, MidiProcessor* processor);

    void enter() override;
    void exit() override;
    void update(Event* event) override;

private:
    MidiSettingsState* state;
    MidiProcessor* processor;
    void render();
    void handle_input(Event* event);
};
