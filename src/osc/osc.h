#include <Oscil.h>
#include <tables/sin2048_int8.h>

static Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aSin1(SIN2048_DATA);

AudioOutput update_audio(void) {
    int8_t osc = aSin1.next();
    return StereoOutput::from8Bit(osc, osc);
}

void event_callback(ProcessorEventType event_type, ProcessorEvent event) {
    // TODO: Implement event callback
}

void osc_init(SignalProcessor* signal_processor) {
    signal_processor->set_update_audio_callback(update_audio);
    signal_processor->set_event_callback(event_callback);

    aSin1.setFreq(440);
}
