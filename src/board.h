#pragma once

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
const int PWM_0_PIN = 33;
const int PWM_1_PIN = 25;
const int PWM_2_PIN = 26;
const int ADC_0 = 36;
const int ADC_1 = 37;
const int SYNC_IN = 18;
const int SYNC_OUT = 19;
const int NEO_PIXEL_PIN = 23;

// PWM parameters
const uint32_t PWM_FREQ = 78125;
const uint8_t  PWM_RESOLUTION = 10;
const uint32_t PWM_MAX_VAL = (1 << PWM_RESOLUTION) - 1;

// PWM channels
const int PWM_0 = 0;
const int PWM_1 = 1;
const int PWM_2 = 2; 