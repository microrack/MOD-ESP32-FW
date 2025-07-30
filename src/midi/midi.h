#pragma once

#include <MIDI.h>
#include "../board.h"
#include "../screen_switcher.h"
#include "../urack_types.h"
#include "midi_info.h"
#include "midi_processor.h"
#include "midi_settings.h"
#include "midi_settings_state.h"

class MidiRoot : public ScreenInterface {
public:
    MidiRoot(Display* display);
    void begin(void);
    void enter() override;
    void exit() override;
    void update(Event* event) override;

private:
    MidiInfo midi_info;
    MidiSettings midi_settings;
    ScreenInterface* midi_screens[2];
    ScreenSwitcher screen_switcher;
    MidiSettingsState state;
    MidiProcessor processor;
};
