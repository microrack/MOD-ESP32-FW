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
#include "midi/ble_midi.h"
#include "midi/usb_midi.h"
#include "signal_processor/signal_processor.h"
#include "screen_switcher.h"
#include "testmode.h"

// Create display object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Create input handler
Input input_handler;

// Create MIDI settings state and processor
MidiSettingsState midi_settings_state;
SignalProcessor signal_processor(&midi_settings_state);

// Create screen objects
OscilloscopeRoot oscilloscope_screen(&display);
MidiRoot midi_screen(&display, &midi_settings_state, &signal_processor);

// Create screen array and switcher
ScreenInterface* screens[] = {&oscilloscope_screen, &midi_screen};
const size_t screen_count = sizeof(screens) / sizeof(screens[0]);
ScreenSwitcher screen_switcher(screens, screen_count);

// NeoPixel setup
Adafruit_NeoPixel pixels(1, NEO_PIXEL_PIN, NEO_GRB + NEO_KHZ800);

void osc_init(SignalProcessor* signal_processor);

void setup() {
    pinMode(OUT_CHANNELS[OutChannelClk].pin, OUTPUT);
    pinMode(OUT_CHANNELS[OutChannelRst].pin, OUTPUT);
    
    // Initialize serial
    Serial.begin(SERIAL_BAUDRATE);

    Serial.printf("setup\n");

    // Initialize display
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("SSD1306 allocation failed"));
        for(;;);
    }
    display.setRotation(2);
    display.clearDisplay();
    display.display();

    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    // Initialize input handler
    input_handler = Input();

    midi_settings_state.begin();
    signal_processor.begin();

    // Initialize BLE and USB MIDI
    ble_midi.begin(&signal_processor);
    usb_midi.begin(&signal_processor);
    
    // Restore BLE/USB MIDI state from settings
    if (midi_settings_state.get_bluetooth_enabled()) {
        ble_midi.enable();
    }
    usb_midi.enable();

    // Check for test mode
    nvs_handle_t nvs_handle;
    err = nvs_open("testmode", NVS_READWRITE, &nvs_handle);
    if (err == ESP_OK) {
        uint8_t testmode_val = 0;
        err = nvs_get_u8(nvs_handle, "testmode", &testmode_val);
        Serial.printf("testmode: nvs_get_u8 err=0x%x, val=%d\n", err, testmode_val);
        if (err == ESP_OK && testmode_val == 1 && false) {
            Serial.println("Entering test mode");
            bool test_completed = test_mode(&display, &input_handler, &signal_processor);
            if (test_completed) {
                // Write 0 to NVS and restart
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

    // Initialize NeoPixel
    pixels.begin();
    pixels.setBrightness(20);
    for(size_t i = 0; i < 4; i++) {
        pixels.setPixelColor(i, pixels.Color(2, 2, 0));
    }
    pixels.show();

    osc_init(&signal_processor);

    // Initialize MIDI screen components
    midi_screen.begin();

    screen_switcher.set_screen(1);
}

void loop() {
    // Get input events
    Event event = input_handler.get_inputs();

    static bool screen_switched = false;

    // Handle screen switching with a state machine approach
    if (event.button_a == ButtonRelease) {
        screen_switched = false;
    } else if (event.button_a == ButtonHold && event.button_a_ms > 400 && !screen_switched) {
        // Switch screen only if the button was released before and hasn't switched screens in this hold session
        screen_switcher.set_screen(screen_switcher.get_next());
        screen_switched = true;
    }

    // Update current screen
    screen_switcher.update(&event);
    
    // Update USB MIDI (process incoming messages)
    usb_midi.update();

    // Event::print(event);
}
