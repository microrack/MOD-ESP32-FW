#pragma once

#include <Arduino.h>

// Forward declaration
class SignalProcessor;

class UsbMidi {
public:
    UsbMidi();
    ~UsbMidi();
    
    void begin(SignalProcessor* processor);
    void enable();
    void disable();
    bool is_enabled() const;
    void update();  // Call in loop to process USB MIDI
    
private:
    SignalProcessor* processor;
    bool enabled;
    bool initialized;
};

extern UsbMidi usb_midi;
