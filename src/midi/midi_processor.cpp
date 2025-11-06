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

    // Initialize PWM using new ESP32 Arduino 3.0 API
    ledcAttach(PWM_0_PIN, PWM_FREQ, PWM_RESOLUTION);
    ledcAttach(PWM_1_PIN, PWM_FREQ, PWM_RESOLUTION);
    ledcAttach(PWM_2_PIN, PWM_FREQ, PWM_RESOLUTION);

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

    // Initialize task handle to nullptr
    midi_task_handle = nullptr;

    for(size_t i = 0; i < PWM_COUNT; i++) {
        last_out[i] = 0;
    }
}

void MidiProcessor::begin(void) {
    // Create MIDI task on second core
    xTaskCreatePinnedToCore(
        midi_task,
        "MIDI_Task",
        4096,
        this,
        1,
        &midi_task_handle,
        1  // Core 1 (second core)
    );
}

void MidiProcessor::midi_task(void* parameter) {
    MidiProcessor* processor = static_cast<MidiProcessor*>(parameter);
    
    while (true) {
        MIDI.read();
        vTaskDelay(pdMS_TO_TICKS(2));  // 2ms delay
    }
}

void MidiProcessor::out_pitch(int ch, int note)
{
    Serial.printf("out_pitch: %d, %d\n", ch, note);

    int v = V_NOTE * note;
    if (v > int(PWM_MAX_VAL))
        return;

    // Map channel to pin for new LEDC API
    int pin;
    switch(ch) {
        case 0: pin = PWM_0_PIN; break;
        case 1: pin = PWM_1_PIN; break;
        case 2: pin = PWM_2_PIN; break;
        default: return;
    }
    ledcWrite(pin, v);
}

void MidiProcessor::out_7bit_value(int pwm_ch, int value)
{
    Serial.printf("out_7bit_value: %d, %d\n", pwm_ch, value);
    
    const int BITS = 7;
    const int SHIFT = PWM_RESOLUTION - BITS;

    int v = value << SHIFT;
    
    // Map channel to pin for new LEDC API
    int pin;
    switch(pwm_ch) {
        case 0: pin = PWM_0_PIN; break;
        case 1: pin = PWM_1_PIN; break;
        case 2: pin = PWM_2_PIN; break;
        default: return;
    }
    ledcWrite(pin, v);
}

void MidiProcessor::out_gate(int pwm_ch, int velocity)
{
    Serial.printf("out_gate: %d, %d\n", pwm_ch, velocity);

    // Map channel to pin for new LEDC API
    int pin;
    switch(pwm_ch) {
        case 0: pin = PWM_0_PIN; break;
        case 1: pin = PWM_1_PIN; break;
        case 2: pin = PWM_2_PIN; break;
        default: return;
    }
    
    if (velocity == 0) {
        ledcWrite(pin, 0);
    } else {
        ledcWrite(pin, PWM_MAX_VAL);
    }
}

void MidiProcessor::handle_note_on(uint8_t channel, uint8_t note, uint8_t velocity) {
    Serial.printf("handle_note_on: %d, %d, %d\n", channel, note, velocity);
    
    if (!is_channel_match(channel)) return;

    if (velocity == 0) {
        handle_note_off(channel, note, velocity);
        return;
    }

    if (!note_history.push(note)) {
        // Note already in use. Skipping.
        return;
    }

    for (int i = 0; i < PWM_COUNT; i++) {
        MidiOutType type = state->get_midi_out_type(i);
        if (type == MidiOutType::MidiOutGate) {
            out_gate(i, velocity);
            last_out[i] = velocity;
        } else if (type == MidiOutType::MidiOutPitch) {
            out_pitch(i, note);
            last_out[i] = note;
        } else if (type == MidiOutType::MidiOutVelocity) {
            out_7bit_value(i, velocity);
            last_out[i] = velocity;
        }
    }
}

void MidiProcessor::handle_note_off(uint8_t channel, uint8_t note, uint8_t velocity) {
    Serial.printf("handle_note_off: %d, %d, %d\n", channel, note, velocity);
    
    if (!is_channel_match(channel)) return;
    
    uint8_t prev_note;
    if (!note_history.pop(note, prev_note)) {
        // Note not in use. Skipping.
        return;
    }

    for (int i = 0; i < PWM_COUNT; i++) {
        if (prev_note == NoteHistory::NO_NOTE) {
            if (state->get_midi_out_type(i) == MidiOutType::MidiOutGate) {
                out_gate(i, 0);
                last_out[i] = 0;
            }
            if (state->get_midi_out_type(i) == MidiOutType::MidiOutVelocity) {
                out_7bit_value(i, 0);
                last_out[i] = 0;
            }
        } else {
            if (state->get_midi_out_type(i) == MidiOutType::MidiOutPitch) {
                // Restore previous note
                out_pitch(i, prev_note);
                last_out[i] = prev_note;
            }
        }
    }
}

void MidiProcessor::handle_cc(uint8_t channel, uint8_t cc, uint8_t value) {
    if (!is_channel_match(channel)) return;

    for (int i = 0; i < PWM_COUNT; i++) {
        if (state->get_midi_out_type(i) == MidiOutType::MidiOutCc0 + cc) {
            out_7bit_value(i, value);
            last_out[i] = value;
        }
    }
}

void MidiProcessor::handle_aftertouch(uint8_t channel, uint8_t value) {
    if (!is_channel_match(channel)) return;

    for (int i = 0; i < PWM_COUNT; i++) {
        if (state->get_midi_out_type(i) == MidiOutType::MidiOutAfterTouch) {
            out_7bit_value(i, value);
            last_out[i] = value;
        }
    }
}

void MidiProcessor::handle_pitchbend(uint8_t channel, uint16_t value) {
    if (!is_channel_match(channel)) return;

    for (int i = 0; i < PWM_COUNT; i++) {
        if (state->get_midi_out_type(i) == MidiOutType::MidiOutPitchBend) {
            // TODO: implement
            last_out[i] = value;
        }
    }
}

void MidiProcessor::handle_clock(void) {
    if (state->get_midi_clk_type() != MidiClkType::MidiClkExt) return;
}

void MidiProcessor::handle_start(void) {
    if (state->get_midi_clk_type() != MidiClkType::MidiClkInt) return;

}

void MidiProcessor::handle_stop(void) {
    if (state->get_midi_clk_type() != MidiClkType::MidiClkInt) return;

}