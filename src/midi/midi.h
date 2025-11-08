#pragma once

#include <MIDI.h>
#include "../board.h"
#include "../urack_types.h"
#include "../screen_switcher.h"
#include "midi_info.h"
#include "midi_settings.h"
#include "midi_settings_state.h"
#include "../signal_processor/signal_processor.h"

enum MidiScreen {
    MidiScreenInfo,
    MidiScreenSettings,
    MidiScreenCount
};

class MidiRoot : public ScreenInterface {
public:
    MidiRoot(Display* display, MidiSettingsState* state, SignalProcessor* processor);
    void begin(void);
    void enter() override;
    void exit() override;
    void update(Event* event) override;

private:
    MidiInfo midi_info;
    MidiSettings midi_settings;
    ScreenInterface* midi_screens[MidiScreen::MidiScreenCount];
    ScreenSwitcher screen_switcher;
    MidiSettingsState* state;
    SignalProcessor* processor;
};
