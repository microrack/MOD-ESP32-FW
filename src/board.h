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

typedef struct {
    int pin;
    bool isPwm;
} OutChannel;

const size_t PWM_COUNT = 3;
const int PWM_0_PIN = 26;
const int PWM_1_PIN = 25;
const int PWM_2_PIN = 33;

const OutChannel OUT_CHANNELS[PWM_COUNT] = {
    {26, true},
    {25, true},
    {33, true},
};

const int ADC_0 = 36;
const int ADC_1 = 37;
const int SYNC_IN = 18;
const int SYNC_OUT = 19;
const int NEO_PIXEL_PIN = 23;
const int MIDI_RX_PIN = 16;
const int MIDI_TX_PIN = 17;

// PWM parameters
const uint32_t PWM_FREQ = 78125;
const uint8_t  PWM_RESOLUTION = 10;
const uint32_t PWM_MAX_VAL = (1 << PWM_RESOLUTION) - 1;

// Debug serial configuration
const unsigned long SERIAL_BAUDRATE = 115200;

// MIDI configuration
const unsigned long MIDI_BAUDRATE = 31250;
const size_t MIDI_OUT_COUNT = 3;
const int MIDI_SETTINGS_EEPROM_ADDR = 0;

// EEPROM
const size_t EEPROM_SIZE = 64;
