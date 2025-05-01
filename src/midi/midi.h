#pragma once

#include "../urack_types.h"
#include <MIDI.h>

class MidiRoot : public ScreenInterface {
public:
    MidiRoot(Display* display);
    void enter() override;
    void exit() override;
    void update(Event* event) override;
}; 