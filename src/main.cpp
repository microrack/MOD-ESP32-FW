/**
 * URack ESP32 Firmware - Main Entry Point
 * 
 * This firmware provides MIDI processing, oscilloscope, and audio synthesis
 * for a modular synthesizer module based on ESP32.
 * 
 * Key components:
 * - Screen-based UI (oscilloscope, MIDI settings)
 * - MIDI to CV/Gate conversion
 * - Real-time audio synthesis via Mozzi
 * - User input via rotary encoder and buttons
 * 
 * See ARCHITECTURE.md for detailed system design.
 */

#include <Arduino.h>
#include <cstdint>
#include <SPI.h>
#include <Wire.h>
#include <nvs_flash.h>
#include <nvs.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>
#include "board.h"
#include "input/input.h"
#include "oscilloscope/oscilloscope.h"
#include "midi/midi.h"
#include "midi/midi_settings_state.h"
#include "signal_processor/signal_processor.h"
#include "screen_switcher.h"
#include "testmode.h"

// Display: 128x64 OLED via I2C (address 0x3C)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// User input: rotary encoder and two buttons
Input input_handler;

// MIDI: Persistent settings and signal processing (MIDI → CV/Gate/Audio)
MidiSettingsState midi_settings_state;
SignalProcessor signal_processor(&midi_settings_state);

// UI Screens: Modular screen system for oscilloscope and MIDI
OscilloscopeRoot oscilloscope_screen(&display);
MidiRoot midi_screen(&display, &midi_settings_state, &signal_processor);

// Screen management: Navigate between screens using button A
ScreenInterface* screens[] = {&oscilloscope_screen, &midi_screen};
const size_t screen_count = sizeof(screens) / sizeof(screens[0]);
ScreenSwitcher screen_switcher(screens, screen_count);

// Status LED: Single RGB NeoPixel for visual feedback
Adafruit_NeoPixel pixels(1, NEO_PIXEL_PIN, NEO_GRB + NEO_KHZ800);

// Forward declaration for audio synthesis initialization
void osc_init(SignalProcessor* signal_processor);

/**
 * Setup: Hardware initialization and boot sequence
 * 
 * Initializes all peripherals and subsystems:
 * 1. GPIO outputs (clock/reset)
 * 2. Serial debugging (115200 baud)
 * 3. OLED display (I2C, rotated 180°)
 * 4. NVS (Non-Volatile Storage) for persistent settings
 * 5. User input (encoder and buttons)
 * 6. MIDI settings and signal processor
 * 7. Test mode check (factory testing)
 * 8. Status LED
 * 9. Audio synthesis (Mozzi)
 * 10. UI screens
 */
void setup() {
    // Configure clock and reset outputs for sequencing
    pinMode(OUT_CHANNELS[OutChannelClk].pin, OUTPUT);
    pinMode(OUT_CHANNELS[OutChannelRst].pin, OUTPUT);
    
    // Initialize serial for debugging (115200 baud)
    Serial.begin(SERIAL_BAUDRATE);
    Serial.printf("setup\n");

    // Initialize OLED display (128x64, I2C address 0x3C)
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("SSD1306 allocation failed"));
        for(;;);  // Halt if display initialization fails
    }
    display.setRotation(2);  // Rotate 180° for correct orientation
    display.clearDisplay();
    display.display();

    // Initialize NVS (Non-Volatile Storage) for persistent configuration
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    // Initialize input handler (encoder and buttons)
    input_handler = Input();

    // Load MIDI settings from NVS and start signal processor
    midi_settings_state.begin();
    signal_processor.begin();

    // Check for test mode flag (factory hardware testing)
    // Test mode validates all I/O and self-clears on successful completion
    nvs_handle_t nvs_handle;
    err = nvs_open("testmode", NVS_READWRITE, &nvs_handle);
    if (err == ESP_OK) {
        uint8_t testmode_val = 0;
        err = nvs_get_u8(nvs_handle, "testmode", &testmode_val);
        Serial.printf("testmode: nvs_get_u8 err=0x%x, val=%d\n", err, testmode_val);
        if (err == ESP_OK && testmode_val == 1) {
            Serial.println("Entering test mode");
            bool test_completed = test_mode(&display, &input_handler, &signal_processor);
            if (test_completed) {
                // Clear test mode flag and restart
                nvs_set_u8(nvs_handle, "testmode", 0);
                nvs_commit(nvs_handle);
                nvs_close(nvs_handle);
                ESP.restart();
            }
        } else {
            Serial.printf("testmode: not entering test mode (err=0x%x, val=%d)\n", err, testmode_val);
        }
        nvs_close(nvs_handle);
    } else {
        Serial.printf("testmode: failed to open NVS namespace, err=0x%x\n", err);
    }

    // Initialize status LED (single RGB NeoPixel)
    pixels.begin();
    pixels.setBrightness(20);  // Low brightness to avoid eye strain
    for(size_t i = 0; i < 4; i++) {
        pixels.setPixelColor(i, pixels.Color(2, 2, 0));  // Dim yellow
    }
    pixels.show();

    // Initialize audio synthesis (Mozzi library)
    osc_init(&signal_processor);

    // Initialize MIDI UI screen
    midi_screen.begin();

    // Start with MIDI screen (index 1)
    screen_switcher.set_screen(1);
}

/**
 * Main Loop: UI update and event processing
 * 
 * Runs continuously to:
 * 1. Poll user input (encoder rotation, button presses)
 * 2. Handle screen switching (long press on button A)
 * 3. Update active screen (redraw display, process input)
 * 
 * Note: Audio synthesis and MIDI processing run in separate
 * tasks/interrupts and don't block the main loop.
 */
void loop() {
    // Poll hardware inputs (encoder, buttons)
    Event event = input_handler.get_inputs();

    static bool screen_switched = false;

    // Screen switching: Long press (>400ms) on button A cycles between screens
    // State machine prevents multiple switches during a single hold
    if (event.button_a == ButtonRelease) {
        screen_switched = false;  // Reset flag on release
    } else if (event.button_a == ButtonHold && event.button_a_ms > 400 && !screen_switched) {
        // Long press detected: switch to next screen
        screen_switcher.set_screen(screen_switcher.get_next());
        screen_switched = true;  // Prevent repeated switching during this hold
    }

    // Update active screen with input events
    // Screen handles encoder rotation, short button presses, and display redraw
    screen_switcher.update(&event);

    // Debug: Uncomment to print input events to serial
    // Event::print(event);
}
