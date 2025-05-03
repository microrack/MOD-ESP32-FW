#pragma once

#include "../urack_types.h"

class MidiInfo : public ScreenInterface {
public:
    MidiInfo(Display* display);
    void enter() override;
    void exit() override;
    void update(Event* event) override;
};
