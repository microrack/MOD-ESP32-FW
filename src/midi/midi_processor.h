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
    void handle_pitchbend(uint8_t channel, int value);
    void handle_clock(void);
    void handle_start(void);
    void handle_stop(void);

    uint8_t last_out[OutChannelCount];

private:
    static constexpr float PWM_NOTE_SCALE = (1 << PWM_RESOLUTION) / (12 * 10.99); // 10.99 Vpp, 12 notes per octave (1 V/oct)
    static const int PWM_ZERO_OFFSET = 498; // 0 V at MIDDLE_NOTE
    static const int MIDDLE_NOTE = 60; // C4 (middle C)
    static constexpr float PITCHBEND_RANGE_SEMITONES = 2.0f; // Standard MIDI pitchbend range in semitones

    MidiSettingsState* state;
    NoteHistory note_history[MIDI_CHANNEL_COUNT];
    TaskHandle_t midi_task_handle;
    int pitchbend[MIDI_CHANNEL_COUNT]; // Raw pitchbend value per channel

    void out_gate(int pwm_ch, int velocity);
    void out_pitch(int pwm_ch, int note, int pitchbend_value = 0);
    void out_7bit_value(int pwm_ch, int value);
    static void midi_task(void* parameter);

    inline bool is_global_channel_match(uint8_t channel) const {
        return (state->get_midi_channel() == channel) ||
               (state->get_midi_channel() == MidiChannelAll);
    }

    inline bool is_out_channel_match(int out_channel, uint8_t channel) const {
        if(state->get_midi_out_channel(out_channel) == MidiChannelUnchanged) return is_global_channel_match(channel);

        return (state->get_midi_out_channel(out_channel) == channel) ||
               (state->get_midi_out_channel(out_channel) == MidiChannelAll);
    }
};
