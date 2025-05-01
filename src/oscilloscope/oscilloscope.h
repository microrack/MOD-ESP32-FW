#pragma once

#include "../urack_types.h"

class OscilloscopeRoot : public ScreenInterface {
public:
    OscilloscopeRoot(Display* display) : ScreenInterface(display) {}
    void enter() override;
    void exit() override;
    void update(Event* event) override;
}; 