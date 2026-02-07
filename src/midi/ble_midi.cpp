#include "ble_midi.h"
#include "../signal_processor/signal_processor.h"
#include <BLEMidi.h>
#include <NimBLEDevice.h>

// Global instance
BleMidi ble_midi;

// Static pointer for callbacks
static SignalProcessor* s_processor = nullptr;
static BleMidi* s_ble_midi = nullptr;

static uint8_t normalize_ble_channel(uint8_t raw_channel) {
    return (raw_channel & 0x0F) + 1; // MIDI channels are transmitted 0-15; processor expects 1-16
}

static void log_ble_channel_event(const char* type, uint8_t raw_channel, uint8_t channel, uint8_t data1, uint8_t data2, uint16_t timestamp, bool processed) {
    if (!DEBUG_BLE_MIDI) return;
    Serial.printf("[BLE MIDI] %s raw_ch=%u ch=%u data1=%u data2=%u ts=%u%s\n",
                  type,
                  raw_channel,
                  channel,
                  data1,
                  data2,
                  timestamp,
                  processed ? "" : " (ignored)");
}

static void log_ble_system_event(const char* type, bool processed) {
    if (!DEBUG_BLE_MIDI) return;
    Serial.printf("[BLE MIDI] %s%s\n", type, processed ? "" : " (ignored)");
}

BleMidi::BleMidi() 
    : processor(nullptr), enabled(false), connected(false), initialized(false) {
}

BleMidi::~BleMidi() {
    disable();
}

void BleMidi::begin(SignalProcessor* proc) {
    processor = proc;
    s_processor = proc;
    s_ble_midi = this;
}

void BleMidi::enable() {
    if (enabled) return;
    
    if (!initialized) {
        // Reduce pairing friction on Android: no bonding, no MITM, no IO cap
        NimBLEDevice::setSecurityAuth(false, false, false);
        NimBLEDevice::setSecurityIOCap(BLE_HS_IO_NO_INPUT_OUTPUT);
        NimBLEDevice::setPower(ESP_PWR_LVL_P9); // max TX power for better discovery

        BLEMidiServer.begin("microrack BLE");
        BLEMidiServer.setOnConnectCallback(onConnect);
        BLEMidiServer.setOnDisconnectCallback(onDisconnect);
        BLEMidiServer.setNoteOnCallback(onNoteOn);
        BLEMidiServer.setNoteOffCallback(onNoteOff);
        BLEMidiServer.setControlChangeCallback(onControlChange);

        // Ensure the advertised name/UUID are visible to picky scanners (e.g., Android Bluetooth MIDI Connect)
        auto* adv = BLEDevice::getAdvertising();
        if (adv) {
            adv->setScanResponse(true);
            adv->setMinPreferred(6);  // faster connection
            adv->setMaxPreferred(12);
        }

        // Note: ESP32-BLE-MIDI library doesn't support pitch bend callback directly
        initialized = true;
    }
    
    enabled = true;
    Serial.println("BLE MIDI enabled");
}

void BleMidi::disable() {
    if (!enabled) return;
    
    // Note: BLEMidi library doesn't have a clean way to fully disable,
    // but we can stop advertising and ignore callbacks
    enabled = false;
    connected = false;
    Serial.println("BLE MIDI disabled");
}

bool BleMidi::is_enabled() const {
    return enabled;
}

bool BleMidi::is_connected() const {
    return connected && enabled;
}

void BleMidi::onConnect() {
    if (s_ble_midi) {
        s_ble_midi->connected = true;
    }
    Serial.println("BLE MIDI connected");
}

void BleMidi::onDisconnect() {
    if (s_ble_midi) {
        s_ble_midi->connected = false;
    }
    Serial.println("BLE MIDI disconnected");
}

void BleMidi::onNoteOn(uint8_t channel, uint8_t note, uint8_t velocity, uint16_t timestamp) {
    bool ready = s_ble_midi && s_ble_midi->enabled && s_processor;
    uint8_t processor_channel = normalize_ble_channel(channel);

    log_ble_channel_event("NoteOn", channel, processor_channel, note, velocity, timestamp, ready);
    if (!ready) return;
    s_processor->handle_note_on(processor_channel, note, velocity);
}

void BleMidi::onNoteOff(uint8_t channel, uint8_t note, uint8_t velocity, uint16_t timestamp) {
    bool ready = s_ble_midi && s_ble_midi->enabled && s_processor;
    uint8_t processor_channel = normalize_ble_channel(channel);

    log_ble_channel_event("NoteOff", channel, processor_channel, note, velocity, timestamp, ready);
    if (!ready) return;
    s_processor->handle_note_off(processor_channel, note, velocity);
}

void BleMidi::onControlChange(uint8_t channel, uint8_t controller, uint8_t value, uint16_t timestamp) {
    bool ready = s_ble_midi && s_ble_midi->enabled && s_processor;
    uint8_t processor_channel = normalize_ble_channel(channel);

    log_ble_channel_event("ControlChange", channel, processor_channel, controller, value, timestamp, ready);
    if (!ready) return;
    s_processor->handle_cc(processor_channel, controller, value);
}

void BleMidi::onClock() {
    bool ready = s_ble_midi && s_ble_midi->enabled && s_processor;
    log_ble_system_event("Clock", ready);
    if (!ready) return;
    s_processor->handle_clock();
}

void BleMidi::onStart() {
    bool ready = s_ble_midi && s_ble_midi->enabled && s_processor;
    log_ble_system_event("Start", ready);
    if (!ready) return;
    s_processor->handle_start();
}

void BleMidi::onStop() {
    bool ready = s_ble_midi && s_ble_midi->enabled && s_processor;
    log_ble_system_event("Stop", ready);
    if (!ready) return;
    s_processor->handle_stop();
}
