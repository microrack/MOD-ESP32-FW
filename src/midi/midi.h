#pragma once

#include "../urack_types.h"

class MidiRoot : public ScreenInterface {
public:
    MidiRoot(Display* display) : ScreenInterface(display) {}
    void enter() override;
    void exit() override;
    void update(Event* event) override;
}; 