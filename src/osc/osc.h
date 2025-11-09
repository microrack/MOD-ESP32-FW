#include <Oscil.h>
#include <tables/waveshape_chebyshev_5th_256_int8.h>
#include <mozzi_midi.h>

const bool DEBUG_OSC = true;
const int NUM_OSCS = 5;

int pitchBend[MOZZI_AUDIO_CHANNELS];

struct Osc {
    Oscil <CHEBYSHEV_5TH_256_NUM_CELLS, AUDIO_RATE> oscil;
    uint8_t note;
    uint8_t velocity;

    Osc() : oscil(CHEBYSHEV_5TH_256_DATA), note(0), velocity(0) {}

    void setFreq(int pitchBend) {
        oscil.setFreq(
            Q16n16_to_float(Q16n16_mtof(Q8n0_to_Q16n16(note))) * pow(2.0f, pitchBend * SignalProcessor::PITCHBEND_RANGE_SEMITONES / 12.0f)
        );
    }
};

static Osc oscs[MOZZI_AUDIO_CHANNELS][NUM_OSCS];

AudioOutput update_audio(void) {
    float l = 0;
    float r = 0;
    for(int i = 0; i < NUM_OSCS; i++) {
        l += oscs[0][i].oscil.next() * (float)oscs[0][i].velocity / 127.0f;
        r += oscs[1][i].oscil.next() * (float)oscs[1][i].velocity / 127.0f;
    }
    return StereoOutput::from8Bit((int)(l / NUM_OSCS), (int)(r / NUM_OSCS));
}

void event_callback(ProcessorEventType event_type, ProcessorEvent event) {
    // print note on event
    if (event_type == EventNoteOn) {
        if(DEBUG_OSC) Serial.printf("note on: %d, %d, %d id: %d\n", event.note.channel, event.note.note, event.note.velocity, event.note.id);

        if(event.note.id < NUM_OSCS) {
            oscs[event.note.channel][event.note.id].note = event.note.note;
            oscs[event.note.channel][event.note.id].velocity = event.note.velocity;
            oscs[event.note.channel][event.note.id].setFreq(pitchBend[event.note.channel]);
        }
    }
    // print note off event
    if (event_type == EventNoteOff) {
        if(DEBUG_OSC) Serial.printf("note off: %d, %d, %d id: %d\n", event.note.channel, event.note.note, event.note.velocity, event.note.id);
        if(event.note.id < NUM_OSCS) {
            oscs[event.note.channel][event.note.id].velocity = 0;
        }
    }

    // print cc event
    if (event_type == EventCc) {
        if(DEBUG_OSC) Serial.printf("cc: %d, %d, %d\n", event.cc.channel, event.cc.cc, event.cc.value);
    }

    if (event_type == EventPitchBend) {
        pitchBend[event.pitchbend.channel] = event.pitchbend.value;
        for(int i = 0; i < NUM_OSCS; i++) {
            oscs[event.pitchbend.channel][i].setFreq(pitchBend[event.pitchbend.channel]);
        }
    }

    if(DEBUG_OSC) {
        if(event_type == EventNoteOff || event_type == EventNoteOn) {
            // print list of enabled oscs
            Serial.print("enabled L oscs: ");
            for(int i = 0; i < NUM_OSCS; i++) {
                if(oscs[0][i].velocity > 0) {
                    Serial.print(i);
                    Serial.print(":");
                    Serial.print(oscs[0][i].note);
                    Serial.print(":");
                    Serial.print(oscs[0][i].velocity);
                    Serial.print(" ");
                }
            }
            Serial.println();
            Serial.print("enabled R oscs: ");
            for(int i = 0; i < NUM_OSCS; i++) {
                if(oscs[1][i].velocity > 0) {
                    Serial.print(i);
                    Serial.print(":");
                    Serial.print(oscs[1][i].note);
                    Serial.print(":");
                    Serial.print(oscs[1][i].velocity);
                    Serial.print(" ");
                }
            }
            Serial.println();
        }
    }
}

void osc_init(SignalProcessor* signal_processor) {
    signal_processor->set_update_audio_callback(update_audio);
    signal_processor->set_event_callback(event_callback);
}
