/**
 * Hardware Configuration and Pin Definitions
 * 
 * This file centralizes all hardware-specific constants for the URack ESP32 module.
 * 
 * Key subsystems:
 * - OLED Display (128x64, SSD1306, I2C)
 * - User Input (rotary encoder, 2 buttons)
 * - MIDI I/O (UART, 31250 baud)
 * - Audio/CV Outputs (Mozzi audio, PWM CV, GPIO gates)
 * - Analog Inputs (2 channels for oscilloscope)
 * - Status LED (NeoPixel RGB)
 * 
 * Hardware reference: ESP32 DevKit with custom URack board
 * See ARCHITECTURE.md for detailed hardware architecture.
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

// ============================================================================
// OLED Display Configuration (SSD1306, I2C)
// ============================================================================
#define SCREEN_WIDTH 128        // Display width in pixels
#define SCREEN_HEIGHT 64        // Display height in pixels
#define OLED_RESET -1          // No reset pin (shared with ESP32 reset)
#define SCREEN_ADDRESS 0x3C    // I2C address (fixed for this display model)

// ============================================================================
// User Input Pins
// ============================================================================
const int BUTTON_A = 38;       // Additional user button (screen switching)
const int ENCODER_SW = 39;     // Encoder push button
const int ENCODER_A = 34;      // Encoder quadrature signal A
const int ENCODER_B = 35;      // Encoder quadrature signal B

// ============================================================================
// Audio/CV Output Configuration
// ============================================================================
// Output pins for audio channels (used by Mozzi library)
#define OUT_CHANNEL_A_PIN 26   // Left audio channel (Mozzi PWM)
#define OUT_CHANNEL_B_PIN 25   // Right audio channel (Mozzi PWM)

// Output channel types determine how signals are generated
enum OutChannelType {
    OutTypeMozzi,              // Real-time audio synthesis via Mozzi library
    OutTypePwm,                // PWM-based CV output (e.g., 1V/octave pitch)
    OutTypeGpio                // Digital gate/trigger outputs
};

// Output channel types determine how signals are generated
enum OutChannelType {
    OutTypeMozzi,              // Real-time audio synthesis via Mozzi library
    OutTypePwm,                // PWM-based CV output (e.g., 1V/octave pitch)
    OutTypeGpio                // Digital gate/trigger outputs
};

// Output channel configuration structure
typedef struct {
    int pin;                   // ESP32 GPIO pin number
    OutChannelType type;       // Output signal type
} OutChannel;

// Output channel enumeration for array indexing
enum OutChannelName {
    OutChannelA,               // Mozzi audio left (pin 26)
    OutChannelB,               // Mozzi audio right (pin 25)
    OutChannelC,               // PWM CV output (pin 33, 1V/octave)
    OutChannelClk,             // Clock/trigger output (pin 12)
    OutChannelRst,             // Reset/trigger output (pin 13)
    OutChannelCount,           // Total number of output channels
};

// Output channel definitions
// Note: Channels 0 and 1 use virtual pin numbers for Mozzi (not actual GPIO)
const OutChannel OUT_CHANNELS[OutChannelCount] = {
    {0, OutTypeMozzi},         // Left audio: Mozzi manages actual pin (26)
    {1, OutTypeMozzi},         // Right audio: Mozzi manages actual pin (25)
    {33, OutTypePwm},          // CV output C: PWM for pitch CV
    {12, OutTypeGpio},         // Clock output: Digital gate/trigger
    {13, OutTypeGpio},         // Reset output: Digital gate/trigger
};

// ============================================================================
// Analog and Digital I/O Pins
// ============================================================================
const int ADC_0 = 36;          // Analog input 0 (oscilloscope channel 1)
const int ADC_1 = 37;          // Analog input 1 (oscilloscope channel 2)
const int SYNC_IN = 18;        // External sync input
const int SYNC_OUT = 19;       // Sync output
const int NEO_PIXEL_PIN = 23;  // RGB LED (NeoPixel) status indicator

// ============================================================================
// MIDI Interface Pins
// ============================================================================
const int MIDI_RX_PIN = 16;    // MIDI input (UART RX)
const int MIDI_TX_PIN = 17;    // MIDI output (UART TX)

// ============================================================================
// PWM Configuration for CV Outputs
// ============================================================================
// PWM settings for control voltage generation (1V/octave standard)
#define PWM_FREQ 32768                              // PWM frequency in Hz
#define PWM_RESOLUTION 10                           // PWM bit resolution (10-bit = 0-1023)
const uint32_t PWM_MAX_VAL = (1 << PWM_RESOLUTION) - 1;  // Maximum PWM value (1023)

// ============================================================================
// Serial and MIDI Communication
// ============================================================================
const unsigned long SERIAL_BAUDRATE = 115200;      // Debug serial baud rate
const unsigned long MIDI_BAUDRATE = 31250;         // MIDI standard baud rate
const int MIDI_SETTINGS_EEPROM_ADDR = 0;           // EEPROM address for MIDI settings

// ============================================================================
// Storage Configuration
// ============================================================================
const size_t EEPROM_SIZE = 64;                     // EEPROM allocation in bytes

// ============================================================================
// Debug Flags
// ============================================================================
const bool DEBUG_MIDI_PROCESSOR = false;           // Enable MIDI processing debug output
