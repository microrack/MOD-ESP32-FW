#pragma once

#include "../urack_types.h"

class MidiSettings : public ScreenInterface {
public:
    MidiSettings(Display* display);
    void enter() override;
    void exit() override;
    void update(Event* event) override;
}; 