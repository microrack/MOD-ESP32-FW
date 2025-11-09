#include <Oscil.h>
#include <tables/sin2048_int8.h>

static Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aSin1(SIN2048_DATA);

AudioOutput update_audio(void) {
    int8_t osc = aSin1.next();
    return StereoOutput::from8Bit(osc, osc);
}

void event_callback(ProcessorEventType event_type, ProcessorEvent event) {
    // print note on event
    if (event_type == EventNoteOn) {
        Serial.printf("note on: %d, %d, %d\n", event.note.channel, event.note.note, event.note.velocity);
    }
    // print note off event
    if (event_type == EventNoteOff) {
        Serial.printf("note off: %d, %d, %d\n", event.note.channel, event.note.note, event.note.velocity);
    }
    // print cc event
    if (event_type == EventCc) {
        Serial.printf("cc: %d, %d, %d\n", event.cc.channel, event.cc.cc, event.cc.value);
    }
}

void osc_init(SignalProcessor* signal_processor) {
    signal_processor->set_update_audio_callback(update_audio);
    signal_processor->set_event_callback(event_callback);

    aSin1.setFreq(440);
}
