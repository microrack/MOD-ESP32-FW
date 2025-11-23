#pragma once

#include "../board.h"
#include "../urack_types.h"
#include "../midi/midi_settings_state.h"
#include "../midi/note_history.h"

#include <MozziConfigValues.h>
#define MOZZI_AUDIO_MODE MOZZI_OUTPUT_PWM
#define MOZZI_CONTROL_RATE 1024
#define MOZZI_AUDIO_RATE PWM_FREQ
#define MOZZI_AUDIO_CHANNELS 2
#define MOZZI_AUDIO_PIN_1 OUT_CHANNEL_A_PIN
#define MOZZI_AUDIO_PIN_2 OUT_CHANNEL_B_PIN
#define MOZZI_ANALOG_READ MOZZI_ANALOG_READ_NONE

// Forward declaration for AudioOutput (defined in AudioOutput.h)
struct StereoOutput;
typedef StereoOutput AudioOutput;

enum ProcessorEventType {
    EventControl,
    EventClock,
    EventStart,
    EventStop,
    EventNoteOn,
    EventNoteOff,
    EventCc,
    EventAftertouch,
    EventPitchBend
};

union ProcessorEvent {
    struct {
        // Empty struct for events without arguments
    } empty;
    
    struct {
        uint8_t channel; // 0 or 1 (from OUT_CHANNELS[].pin)
        uint8_t note;
        uint8_t velocity;
        uint8_t id;
    } note;
    
    struct {
        uint8_t channel; // 0 or 1 (from OUT_CHANNELS[].pin)
        uint8_t cc;
        uint8_t value;
    } cc;
    
    struct {
        uint8_t channel; // 0 or 1 (from OUT_CHANNELS[].pin)
        uint8_t value;
    } aftertouch;
    
    struct {
        uint8_t channel; // 0 or 1 (from OUT_CHANNELS[].pin)
        int value;
    } pitchbend;
};

class SignalProcessor {
public:
    SignalProcessor(MidiSettingsState* state);
    MidiSettingsState* state;

    void begin(void);
    void handle_note_on(uint8_t channel, uint8_t note, uint8_t velocity);
    void handle_note_off(uint8_t channel, uint8_t note, uint8_t velocity);
    void handle_cc(uint8_t channel, uint8_t cc, uint8_t value);
    void handle_aftertouch(uint8_t channel, uint8_t value);
    void handle_pitchbend(uint8_t channel, int value);
    void handle_clock(void);
    void handle_start(void);
    void handle_stop(void);
    void clock_routine(void);

    void out_7bit_value(int pwm_ch, int value);

    uint8_t last_out[OutChannelCount];
    uint8_t last_cc[MIDI_CHANNEL_COUNT]; // Last CC number per channel
    int pitchbend[MIDI_CHANNEL_COUNT]; // Raw pitchbend value per channel
    
    bool osc_enabled[2]; // MOZZI_AUDIO_CHANNELS
    int mozzi_out[2]; // MOZZI_AUDIO_CHANNELS
    
    // Callback function types
    typedef AudioOutput (*UpdateAudioCallback)(void);
    typedef void (*EventCallback)(ProcessorEventType, ProcessorEvent);
    
    // Callback setters
    void set_update_audio_callback(UpdateAudioCallback callback) {
        update_audio_callback = callback;
    }
    
    void set_event_callback(EventCallback callback) {
        event_callback = callback;
    }

    UpdateAudioCallback update_audio_callback;
    EventCallback event_callback;

    static constexpr float PITCHBEND_RANGE_SEMITONES = 2.0f; // Standard MIDI pitchbend range in semitones

private:
    static constexpr float PWM_NOTE_SCALE = (1 << PWM_RESOLUTION) / (12 * 10.99); // 10.99 Vpp, 12 notes per octave (1 V/oct)
    static const int PWM_ZERO_OFFSET = 498; // 0 V at MIDDLE_NOTE
    static const int MIDDLE_NOTE = 60; // C4 (middle C)
        
    NoteHistory note_history[MIDI_CHANNEL_COUNT];
    TaskHandle_t midi_task_handle;
    
    // Clock frequency measurement
    static constexpr int CLOCK_TICKS_PER_BEAT = 24; // MIDI clock sends 24 ticks per quarter note
    static constexpr unsigned long MAX_CLOCK_TICK_DURATION = 4; // Maximum clock pulse duration in ticks
    unsigned long clock_last_time;
    int clock_tick_count;
    unsigned long clock_measurement_start;
    unsigned long internal_clock_last_tick_time; // Time of last internal clock tick
    
    void out_gate(int pwm_ch, int velocity);
    void out_pitch(int pwm_ch, int note, int pitchbend_value = 0);
    
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
