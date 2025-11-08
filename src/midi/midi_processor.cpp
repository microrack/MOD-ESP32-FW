#include <MIDI.h>
#include "midi_processor.h"

#include <MozziConfigValues.h>
#define MOZZI_AUDIO_MODE MOZZI_OUTPUT_PWM
#define MOZZI_CONTROL_RATE 1024
#define MOZZI_AUDIO_BITS PWM_RESOLUTION
#define MOZZI_AUDIO_RATE PWM_FREQ
#define MOZZI_AUDIO_CHANNELS 2
#define MOZZI_AUDIO_PIN_1 OUT_CHANNEL_A_PIN
#define MOZZI_AUDIO_PIN_2 OUT_CHANNEL_B_PIN
#include <Mozzi.h>
#include <AudioOutput.h>
#include <Oscil.h>
#include <tables/sin2048_int8.h>

Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aSin1(SIN2048_DATA);

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
    ledcAttach(OUT_CHANNELS[OutChannelA].pin, PWM_FREQ, PWM_RESOLUTION);
    ledcAttach(OUT_CHANNELS[OutChannelB].pin, PWM_FREQ, PWM_RESOLUTION);
    ledcAttach(OUT_CHANNELS[OutChannelC].pin, PWM_FREQ, PWM_RESOLUTION);

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

    for(size_t i = 0; i < OutChannelCount; i++) {
        last_out[i] = 0;
    }

    // Initialize pitchbend to center (0 = no bend)
    for(size_t i = 0; i < MIDI_CHANNEL_COUNT; i++) {
        pitchbend[i] = 0;
        last_cc[i] = 0;
    }

    // Initialize clock measurement
    clock_last_time = 0;
    clock_tick_count = 0;
    clock_measurement_start = 0;
    internal_clock_last_tick_time = 0;
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

MidiProcessor* mozzi_processor = nullptr;

void updateControl() {
    MIDI.read();
    if (mozzi_processor != nullptr) mozzi_processor->clock_routine();
}

AudioOutput updateAudio() {
    int8_t osc = aSin1.next();
    return StereoOutput::from8Bit(osc, osc);
}

void MidiProcessor::midi_task(void* parameter) {
    mozzi_processor = static_cast<MidiProcessor*>(parameter);

    startMozzi();
    aSin1.setFreq(440);

    for (size_t i = 0; i < OutChannelCount; i++) {
        if (mozzi_processor->state->get_midi_out_type(i) == MidiOutType::MidiOutStop) {
            mozzi_processor->out_gate(i, 255);
            mozzi_processor->last_out[i] = 255;
        }
    }
    
    while (true) {
        audioHook();
    }
}

void MidiProcessor::clock_routine(void) {
    unsigned long current_time = millis();
    
    // Generate internal clock ticks if MidiClkInt
    if (state->get_midi_clk_type() == MidiClkType::MidiClkInt) {
        int bpm = state->get_bpm();
        if (bpm > 0) {
            // Calculate tick interval: (60 seconds * 1000 ms) / (bpm * 24 ticks per beat)
            unsigned long tick_interval_ms = (60 * 1000) / (bpm * CLOCK_TICKS_PER_BEAT);
            
            if (internal_clock_last_tick_time == 0) {
                // Initialize on first call
                internal_clock_last_tick_time = current_time;
                clock_measurement_start = current_time;
                clock_tick_count = 0;
            } else {
                // Check if enough time has passed for next tick
                if (current_time - internal_clock_last_tick_time >= tick_interval_ms) {
                    internal_clock_last_tick_time = current_time;
                    clock_tick_count++;
                    
                    // Reset clock_tick_count every CLOCK_TICKS_PER_BEAT ticks (one beat)
                    if (clock_tick_count >= CLOCK_TICKS_PER_BEAT) {
                        clock_tick_count = 0;
                        clock_measurement_start = current_time;
                    }
                }
            }
        }
    }
    
    // Update all clock outputs based on current clock_tick_count
    for (int i = 0; i < OutChannelCount; i++) {
        MidiOutType type = state->get_midi_out_type(i);
        if (state->is_clock_type(type)) {
            int division_ticks = state->get_clock_division_ticks(type);
            if (division_ticks > 0) {
                // Calculate pulse duration: half of period, but not more than MAX_CLOCK_TICK_DURATION
                int pulse_duration = division_ticks / 2;
                if (pulse_duration > MAX_CLOCK_TICK_DURATION) {
                    pulse_duration = MAX_CLOCK_TICK_DURATION;
                }
                
                // Check if gate should be high based on current clock_tick_count
                int phase = clock_tick_count % division_ticks;
                bool should_be_high = phase < pulse_duration;
                
                // Update gate state if needed
                uint8_t target_value = should_be_high ? 255 : 0;
                if (last_out[i] != target_value) {
                    out_gate(i, target_value);
                    last_out[i] = target_value;
                }
            }
        }
    }
}

void MidiProcessor::out_pitch(int ch, int note, int pitchbend_value)
{
    if(ch >= OutChannelCount) return;
    if(ch < 0) return;

    // Calculate pitchbend offset in semitones
    // pitchbend_value: -8192 to +8192, 0 = center (no bend)
    float bend_semitones = (float)pitchbend_value / 8192.0f * PITCHBEND_RANGE_SEMITONES;
    float bent_note = note + bend_semitones;

    if(DEBUG_MIDI_PROCESSOR) Serial.printf("out_pitch: %d, %d (bend: %.2f)\n", ch, note, bend_semitones);

    int v = (bent_note - MIDDLE_NOTE) * PWM_NOTE_SCALE + PWM_ZERO_OFFSET;
    if (v > int(PWM_MAX_VAL))
        return;

    // Map channel to pin for new LEDC API
    int pin = OUT_CHANNELS[ch].pin;
    if(OUT_CHANNELS[ch].isPwm) {
        ledcWrite(pin, v);
    } else {
        return;
    }
}

void MidiProcessor::out_7bit_value(int pwm_ch, int value)
{
    if(pwm_ch >= OutChannelCount) return;
    if(pwm_ch < 0) return;

    if(DEBUG_MIDI_PROCESSOR) Serial.printf("out_7bit_value: %d, %d\n", pwm_ch, value);
    
    int v = map(value, 0, (1 << 7) - 1, PWM_ZERO_OFFSET, PWM_MAX_VAL);
    
    // Map channel to pin for new LEDC API
    int pin = OUT_CHANNELS[pwm_ch].pin;
    if(OUT_CHANNELS[pwm_ch].isPwm) {
        ledcWrite(pin, v);
    } else {
        digitalWrite(pin, v > 0 ? HIGH : LOW);
    }
}

void MidiProcessor::out_gate(int pwm_ch, int velocity)
{
    if(pwm_ch >= OutChannelCount) return;
    if(pwm_ch < 0) return;

    if(DEBUG_MIDI_PROCESSOR) Serial.printf("out_gate: %d, %d\n", pwm_ch, velocity);

    // Map channel to pin for new LEDC API
    int pin = OUT_CHANNELS[pwm_ch].pin;
    if(OUT_CHANNELS[pwm_ch].isPwm) {
        if (velocity == 0) {
            ledcWrite(pin, PWM_ZERO_OFFSET);
        } else {
            ledcWrite(pin, PWM_MAX_VAL);
        }
    } else {
        digitalWrite(pin, velocity > 0 ? HIGH : LOW);
    }
}

void MidiProcessor::handle_note_on(uint8_t channel, uint8_t note, uint8_t velocity) {
    if(DEBUG_MIDI_PROCESSOR) Serial.printf("handle_note_on: %d, %d, %d\n", channel, note, velocity);

    if (velocity == 0) {
        handle_note_off(channel, note, velocity);
        return;
    }

    if (!note_history[channel].push(note)) {
        // Note already in use. Skipping.
        return;
    }

    for (int i = 0; i < OutChannelCount; i++) {
        if (!is_out_channel_match(i, channel)) continue;

        MidiOutType type = state->get_midi_out_type(i);
        if (type == MidiOutType::MidiOutGate) {
            out_gate(i, velocity);
            last_out[i] = velocity;
        } else if (type == MidiOutType::MidiOutPitch) {
            out_pitch(i, note, pitchbend[channel]);
            last_out[i] = note;
        } else if (type == MidiOutType::MidiOutVelocity) {
            out_7bit_value(i, velocity);
            last_out[i] = velocity;
        }
    }
}

void MidiProcessor::handle_note_off(uint8_t channel, uint8_t note, uint8_t velocity) {
    if(DEBUG_MIDI_PROCESSOR) Serial.printf("handle_note_off: %d, %d, %d\n", channel, note, velocity);

    if (!note_history[channel].pop(note)) {
        // Note not in use. Skipping.
        return;
    }

    uint8_t current_note = note_history[channel].get_current();

    for (int i = 0; i < OutChannelCount; i++) {
        if (!is_out_channel_match(i, channel)) continue;
        
        if (current_note == NoteHistory::NO_NOTE) {
            if (state->get_midi_out_type(i) == MidiOutType::MidiOutGate) {
                out_gate(i, 0);
                last_out[i] = 0;
            }
            if (state->get_midi_out_type(i) == MidiOutType::MidiOutVelocity) {
                out_7bit_value(i, 0);
                last_out[i] = 0;
            }
        }

        if (state->get_midi_out_type(i) == MidiOutType::MidiOutPitch) {
            // keep last note CV after note off, with pitchbend applied
            if (current_note != NoteHistory::NO_NOTE) {
                out_pitch(i, current_note, pitchbend[channel]);
                last_out[i] = current_note;
            }
        }
    }
}

void MidiProcessor::handle_cc(uint8_t channel, uint8_t cc, uint8_t value) {
    // Store last CC number for the channel
    last_cc[channel] = cc;

    for (int i = 0; i < OutChannelCount; i++) {
        if (!is_out_channel_match(i, channel)) continue;
        
        if (state->get_midi_out_type(i) == MidiOutType::MidiOutCc0 + cc) {
            out_7bit_value(i, value);
            last_out[i] = value;
        }
    }
}

void MidiProcessor::handle_aftertouch(uint8_t channel, uint8_t value) {
    for (int i = 0; i < OutChannelCount; i++) {
        if (!is_out_channel_match(i, channel)) continue;
        
        if (state->get_midi_out_type(i) == MidiOutType::MidiOutAfterTouch) {
            out_7bit_value(i, value);
            last_out[i] = value;
        }
    }
}

void MidiProcessor::handle_pitchbend(uint8_t channel, int value) {
    if(DEBUG_MIDI_PROCESSOR) Serial.printf("handle_pitchbend: %d, %d\n", channel, value);

    // Store raw pitchbend value
    pitchbend[channel] = value;

    // For all outputs with MidiOutPitch type, update pitch with pitchbend applied
    for (int i = 0; i < OutChannelCount; i++) {
        if (!is_out_channel_match(i, channel)) continue;
        
        if (state->get_midi_out_type(i) == MidiOutType::MidiOutPitch) {
            // Get current note from note history
            uint8_t current_note = note_history[channel].get_current();
            if (current_note != NoteHistory::NO_NOTE) {
                out_pitch(i, current_note, value);
            }
        } else if (state->get_midi_out_type(i) == MidiOutType::MidiOutPitchBend) {
            // Direct pitchbend output (for compatibility)
            out_7bit_value(i, value >> 7); // Use upper 7 bits
            last_out[i] = value >> 7;
        }
    }
}

void MidiProcessor::handle_clock(void) {
    if (state->get_midi_clk_type() != MidiClkType::MidiClkExt) return;

    unsigned long current_time = millis();
    
    // Start measurement on first clock tick
    if (clock_measurement_start == 0) {
        clock_measurement_start = current_time;
        clock_tick_count = 0;
    }
    
    clock_tick_count++;

    // Calculate BPM every CLOCK_TICKS_PER_BEAT ticks (one beat)
    if (clock_tick_count >= CLOCK_TICKS_PER_BEAT) {
        unsigned long elapsed_ms = current_time - clock_measurement_start;
        
        if (elapsed_ms > 0) {
            // BPM = (60 seconds * 1000 ms/sec) / (elapsed_ms ms for one beat)
            // elapsed_ms is already the time for CLOCK_TICKS_PER_BEAT ticks (one beat)
            int bpm = (60 * 1000) / elapsed_ms;
            
            // Clamp to valid range
            if (bpm < state->get_min_bpm()) bpm = state->get_min_bpm();
            if (bpm > state->get_max_bpm()) bpm = state->get_max_bpm();
            
            state->set_bpm(bpm);
        }
        
        // Reset for next measurement
        clock_measurement_start = current_time;
        clock_tick_count = 0;
    }
    
    clock_last_time = current_time;
}

void MidiProcessor::handle_start(void) {
    // Reset clock measurement on start (for external clock)
    if (state->get_midi_clk_type() == MidiClkType::MidiClkExt) {
        clock_measurement_start = 0;
        clock_tick_count = 0;
        
        // Lower all clock outputs
        for (size_t i = 0; i < OutChannelCount; i++) {
            if (state->is_clock_type(state->get_midi_out_type(i))) {
                out_gate(i, 0);
                last_out[i] = 0;
            }
        }
    }

    // Handle MidiOutRun outputs
    for (size_t i = 0; i < OutChannelCount; i++) {
        if (state->get_midi_out_type(i) == MidiOutType::MidiOutRun) {
            out_gate(i, 255);
            last_out[i] = 255;
        }
    }

    // Handle MidiOutStop outputs
    for (size_t i = 0; i < OutChannelCount; i++) {
        if (state->get_midi_out_type(i) == MidiOutType::MidiOutStop) {
            out_gate(i, 0);
            last_out[i] = 0;
        }
    }
}

void MidiProcessor::handle_stop(void) {
    // Handle MidiOutStop outputs
    for (size_t i = 0; i < OutChannelCount; i++) {
        if (state->get_midi_out_type(i) == MidiOutType::MidiOutStop) {
            out_gate(i, 255);
            last_out[i] = 255;
        }
    }
    
    // Handle MidiOutRun outputs
    for (size_t i = 0; i < OutChannelCount; i++) {
        if (state->get_midi_out_type(i) == MidiOutType::MidiOutRun) {
            out_gate(i, 0);
            last_out[i] = 0;
        }
    }
}