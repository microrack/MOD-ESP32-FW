#pragma once

#include <Arduino.h>

// Forward declaration
class SignalProcessor;

class BleMidi {
public:
    BleMidi();
    ~BleMidi();
    
    void begin(SignalProcessor* processor);
    void enable();
    void disable();
    bool is_enabled() const;
    bool is_connected() const;
    
private:
    SignalProcessor* processor;
    bool enabled;
    bool connected;
    bool initialized;
    
    static void onConnect();
    static void onDisconnect();
    static void onNoteOn(uint8_t channel, uint8_t note, uint8_t velocity, uint16_t timestamp);
    static void onNoteOff(uint8_t channel, uint8_t note, uint8_t velocity, uint16_t timestamp);
    static void onControlChange(uint8_t channel, uint8_t controller, uint8_t value, uint16_t timestamp);
    static void onClock();
    static void onStart();
    static void onStop();
};

extern BleMidi ble_midi;
