#include <MIDI.h>
#include "midi_processor.h"
#include "board.h"

// MIDI interface
MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, MIDI);

static MidiProcessor* processor = nullptr;

void handle_note_on(uint8_t channel, uint8_t note, uint8_t velocity) {
    processor->handle_note_on(channel, note, velocity);
}

void handle_note_off(uint8_t channel, uint8_t note, uint8_t velocity) {
    processor->handle_note_off(channel, note, velocity);
}

void handle_cc(uint8_t channel, uint8_t cc, uint8_t value) {
    processor->handle_cc(channel, cc, value);
}

void handle_aftertouch(uint8_t channel, uint8_t value) {
    processor->handle_aftertouch(channel, value);
}

void handle_pitchbend(uint8_t channel, int value) {
    processor->handle_pitchbend(channel, value);
}

void handle_clock(void) {
    processor->handle_clock();
}

void handle_start(void) {
    processor->handle_start();
}

void handle_stop(void) {
    processor->handle_stop();
}

MidiProcessor::MidiProcessor(MidiSettingsState* state)
    : state(state) {

    processor = this;

    // Initialize MIDI
    Serial2.begin(MIDI_BAUDRATE, SERIAL_8N1, MIDI_RX_PIN, MIDI_TX_PIN);
    MIDI.begin(MIDI_CHANNEL_OMNI);

    MIDI.setHandleNoteOn(::handle_note_on);
    MIDI.setHandleNoteOff(::handle_note_off);
    MIDI.setHandleControlChange(::handle_cc);
    MIDI.setHandleAfterTouchChannel(::handle_aftertouch);
    MIDI.setHandlePitchBend(::handle_pitchbend);
    MIDI.setHandleClock(::handle_clock);
    MIDI.setHandleStart(::handle_start);
    MIDI.setHandleStop(::handle_stop);

    ledcAttach(PWM_0_PIN, PWM_FREQ, PWM_RESOLUTION);
    ledcAttach(PWM_1_PIN, PWM_FREQ, PWM_RESOLUTION);
    ledcAttach(PWM_2_PIN, PWM_FREQ, PWM_RESOLUTION);
}

void MidiProcessor::handle_note_on(uint8_t channel, uint8_t note, uint8_t velocity) {
    if (!is_channel_match(channel)) return;

    if (velocity == 0) {
        handle_note_off(channel, note, velocity);
        return;
    }

    for (int i = 0; i < state->get_midi_out_count(); i++) {
        if (state->get_midi_out_type(i) == MidiOutType::MidiOutGate) {
            ledcWrite(PWM_PINS[i], PWM_MAX_VAL);
        }
        if (state->get_midi_out_type(i) == MidiOutType::MidiOutPitch) {
            ledcWrite(PWM_PINS[i], note * PWM_MAX_VAL / 127);
        }
        if (state->get_midi_out_type(i) == MidiOutType::MidiOutVelocity) {
            ledcWrite(PWM_PINS[i], velocity * PWM_MAX_VAL / 127 + PWM_MAX_VAL / 2);
        }
    }
}

void MidiProcessor::handle_note_off(uint8_t channel, uint8_t note, uint8_t velocity) {
    if (!is_channel_match(channel)) return;

    for (int i = 0; i < state->get_midi_out_count(); i++) {
        if (state->get_midi_out_type(i) == MidiOutType::MidiOutGate) {
            ledcWrite(PWM_PINS[i], PWM_MAX_VAL / 2);
        }
    }
}

void MidiProcessor::handle_cc(uint8_t channel, uint8_t cc, uint8_t value) {
    if (!is_channel_match(channel)) return;

    for (int i = 0; i < state->get_midi_out_count(); i++) {
        if (state->get_midi_out_type(i) == MidiOutType::MidiOutCc0 + cc) {
            ledcWrite(PWM_PINS[i], cc * PWM_MAX_VAL / 127 + PWM_MAX_VAL / 2);
        }
    }
}

void MidiProcessor::handle_aftertouch(uint8_t channel, uint8_t value) {
    if (!is_channel_match(channel)) return;

    for (int i = 0; i < state->get_midi_out_count(); i++) {
        if (state->get_midi_out_type(i) == MidiOutType::MidiOutAfterTouch) {
            ledcWrite(PWM_PINS[i], value * PWM_MAX_VAL / 127 + PWM_MAX_VAL / 2);
        }
    }
}

void MidiProcessor::handle_pitchbend(uint8_t channel, uint16_t value) {
    if (!is_channel_match(channel)) return;

    for (int i = 0; i < state->get_midi_out_count(); i++) {
        if (state->get_midi_out_type(i) == MidiOutType::MidiOutPitchBend) {

        }
    }
}

void MidiProcessor::handle_clock(void) {
}

void MidiProcessor::handle_start(void) {

}

void MidiProcessor::handle_stop(void) {

}
