#include <MIDI.h>
#include "midi_processor.h"

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

    // Initialize PWM
    ledcSetup(PWM_0, PWM_FREQ, PWM_RESOLUTION);
    ledcSetup(PWM_1, PWM_FREQ, PWM_RESOLUTION);
    ledcSetup(PWM_2, PWM_FREQ, PWM_RESOLUTION);
    ledcAttachPin(PWM_0_PIN, PWM_0);
    ledcAttachPin(PWM_1_PIN, PWM_1);
    ledcAttachPin(PWM_2_PIN, PWM_2);

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
}

void MidiProcessor::out_pitch(int ch, int note)
{
    int v = V_NOTE * note;
    if (v > int(PWM_MAX_VAL))
        return;

    ledcWrite(ch, v);
}

void MidiProcessor::out_7bit_value(int pwm_ch, int value)
{
    const int BITS = 7;
    const int SHIFT = PWM_RESOLUTION - BITS;

    int v = value << SHIFT;
    ledcWrite(pwm_ch, v);
}

void MidiProcessor::out_gate(int pwm_ch, int velocity)
{
    if (velocity == 0) {
        ledcWrite(pwm_ch, 0);
    } else {
        ledcWrite(pwm_ch, PWM_MAX_VAL);
    }
}

void MidiProcessor::handle_note_on(uint8_t channel, uint8_t note, uint8_t velocity) {
    if (!is_channel_match(channel)) return;

    if (velocity == 0) {
        handle_note_off(channel, note, velocity);
        return;
    }

<<<<<<< HEAD

    if (!note_history.push(note)) {
        // Note already in use. Skipping.
        return;
    }

    for (int i = 0; i < state->get_midi_out_count(); i++) {
        if (state->get_midi_out_type(i) == MidiOutType::MidiOutGate) {
            out_gate(i, velocity);
        }
        if (state->get_midi_out_type(i) == MidiOutType::MidiOutPitch) {
            out_pitch(i, note);
        }
        if (state->get_midi_out_type(i) == MidiOutType::MidiOutVelocity) {
            out_7bit_value(i, velocity);
=======
    for (int i = 0; i < state->get_midi_out_count(); i++) {
        if (state->get_midi_out_type(i) == MidiOutType::MidiOutGate) {

        }
        if (state->get_midi_out_type(i) == MidiOutType::MidiOutPitch) {

        }
        if (state->get_midi_out_type(i) == MidiOutType::MidiOutVelocity) {

>>>>>>> master
        }
    }
}

void MidiProcessor::handle_note_off(uint8_t channel, uint8_t note, uint8_t velocity) {
    if (!is_channel_match(channel)) return;

<<<<<<< HEAD
    uint8_t prev_note;
    if (!note_history.pop(note, prev_note)) {
        // Note not in use. Skipping.
        return;
    }

    for (int i = 0; i < state->get_midi_out_count(); i++) {
        if (prev_note == NoteHistory::NO_NOTE) {
            if (state->get_midi_out_type(i) == MidiOutType::MidiOutGate) {
                out_gate(i, 0);
            }
            if (state->get_midi_out_type(i) == MidiOutType::MidiOutVelocity) {
                out_7bit_value(i, 0);
            }
        } else {
            if (state->get_midi_out_type(i) == MidiOutType::MidiOutPitch) {
                // Restore previous note
                out_pitch(i, prev_note);
            }
=======
    for (int i = 0; i < state->get_midi_out_count(); i++) {
        if (state->get_midi_out_type(i) == MidiOutType::MidiOutGate) {

>>>>>>> master
        }
    }
}

void MidiProcessor::handle_cc(uint8_t channel, uint8_t cc, uint8_t value) {
    if (!is_channel_match(channel)) return;

    for (int i = 0; i < state->get_midi_out_count(); i++) {
        if (state->get_midi_out_type(i) == MidiOutType::MidiOutCc0 + cc) {
<<<<<<< HEAD
            out_7bit_value(i, value);
=======

>>>>>>> master
        }
    }
}

void MidiProcessor::handle_aftertouch(uint8_t channel, uint8_t value) {
    if (!is_channel_match(channel)) return;

    for (int i = 0; i < state->get_midi_out_count(); i++) {
        if (state->get_midi_out_type(i) == MidiOutType::MidiOutAfterTouch) {
<<<<<<< HEAD
            out_7bit_value(i, value);
=======

>>>>>>> master
        }
    }
}

void MidiProcessor::handle_pitchbend(uint8_t channel, uint16_t value) {
    if (!is_channel_match(channel)) return;

    for (int i = 0; i < state->get_midi_out_count(); i++) {
        if (state->get_midi_out_type(i) == MidiOutType::MidiOutPitchBend) {
<<<<<<< HEAD
            // TODO: implement
=======

>>>>>>> master
        }
    }
}

void MidiProcessor::handle_clock(void) {
<<<<<<< HEAD
    if (state->get_midi_clk_type() != MidiClkType::MidiClkExt) return;

=======
>>>>>>> master
}

void MidiProcessor::handle_start(void) {
    if (state->get_midi_clk_type() != MidiClkType::MidiClkInt) return;

}

void MidiProcessor::handle_stop(void) {
    if (state->get_midi_clk_type() != MidiClkType::MidiClkInt) return;

}
