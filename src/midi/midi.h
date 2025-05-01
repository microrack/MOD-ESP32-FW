#pragma once

#include "../urack_types.h"
#include <MIDI.h>

class MidiRoot : public ScreenInterface {
public:
    MidiRoot(Display* display) : ScreenInterface(display) {}
    void enter() override;
    void exit() override;
    void update(Event* event) override;

    // MIDI handlers
    static void handle_note_on(byte channel, byte note, byte velocity);
    static void handle_note_off(byte channel, byte note, byte velocity);
    static void handle_control_change(byte channel, byte number, byte value);
}; 