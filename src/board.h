#pragma once

#include <stddef.h>
#include <stdint.h>

// OLED display configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

// Pin definitions
const int BUTTON_A = 38;
const int ENCODER_SW = 39;
const int ENCODER_A = 34;
const int ENCODER_B = 35;

#define OUT_CHANNEL_A_PIN 26
#define OUT_CHANNEL_B_PIN 25

enum OutChannelType {
    OutTypeMozzi,
    OutTypePwm,
    OutTypeGpio
};

typedef struct {
    int pin;
    OutChannelType type;
} OutChannel;

enum OutChannelName {
    OutChannelA,
    OutChannelB,
    OutChannelC,
    OutChannelClk,
    OutChannelRst,
    OutChannelCount,
};

const OutChannel OUT_CHANNELS[OutChannelCount] = {
    {0, OutTypeMozzi}, // pass value through mozzi left
    {1, OutTypeMozzi}, // pass value through mozzi right
    {33, OutTypePwm},
    {12, OutTypeGpio}, // clk
    {13, OutTypeGpio}, // reset
};

const int ADC_0 = 36;
const int ADC_1 = 37;
const int SYNC_IN = 18;
const int SYNC_OUT = 19;
const int NEO_PIXEL_PIN = 23;
const int MIDI_RX_PIN = 16;
const int MIDI_TX_PIN = 17;

// PWM parameters
#define PWM_FREQ 32768
#define PWM_RESOLUTION 10
const uint32_t PWM_MAX_VAL = (1 << PWM_RESOLUTION) - 1;

// Debug serial configuration
const unsigned long SERIAL_BAUDRATE = 115200;

// MIDI configuration
const unsigned long MIDI_BAUDRATE = 31250;
const int MIDI_SETTINGS_EEPROM_ADDR = 0;

// EEPROM
const size_t EEPROM_SIZE = 64;

const bool DEBUG_MIDI_PROCESSOR = false;
