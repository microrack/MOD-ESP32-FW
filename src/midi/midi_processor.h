#pragma once

#include "../board.h"
#include "../urack_types.h"
#include "midi_settings_state.h"
#include "note_history.h"

class MidiProcessor {
public:
    MidiProcessor(MidiSettingsState* state);

    void begin(void);
    void handle_note_on(uint8_t channel, uint8_t note, uint8_t velocity);
    void handle_note_off(uint8_t channel, uint8_t note, uint8_t velocity);
    void handle_cc(uint8_t channel, uint8_t cc, uint8_t value);
    void handle_aftertouch(uint8_t channel, uint8_t value);
    void handle_pitchbend(uint8_t channel, uint16_t value);
    void handle_clock(void);
    void handle_start(void);
    void handle_stop(void);

    uint8_t last_out[PWM_COUNT];

private:
    static const int V_NOTE = 1024 / (12 * 10);

    MidiSettingsState* state;
    NoteHistory note_history;
    TaskHandle_t midi_task_handle;

    void out_gate(int pwm_ch, int velocity);
    void out_pitch(int pwm_ch, int note);
    void out_7bit_value(int pwm_ch, int value);
    static void midi_task(void* parameter);

    inline bool is_channel_match(uint8_t channel) const {
        return (state->get_midi_channel() == channel) ||
               (state->get_midi_channel() == MidiChannelAll);
    }
};
