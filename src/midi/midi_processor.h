#pragma once

#include "../board.h"
#include "../urack_types.h"
#include "midi_settings_state.h"

class MidiProcessor {
public:
    MidiProcessor(MidiSettingsState* state);

    void handle_note_on(uint8_t channel, uint8_t note, uint8_t velocity);
    void handle_note_off(uint8_t channel, uint8_t note, uint8_t velocity);
    void handle_cc(uint8_t channel, uint8_t cc, uint8_t value);
    void handle_aftertouch(uint8_t channel, uint8_t value);
    void handle_pitchbend(uint8_t channel, uint16_t value);
    void handle_clock(void);
    void handle_start(void);
    void handle_stop(void);

private:
    MidiSettingsState* state;

    inline bool is_channel_match(uint8_t channel) const {
        return (state->get_midi_channel() == channel) ||
               (state->get_midi_channel() == MidiChannelAll);
    }
};
