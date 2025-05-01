#include <Arduino.h>
#include <cstdint>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Encoder.h>
#include <Adafruit_NeoPixel.h>
#include <MIDI.h>
#include "board.h"
#include "input/input.h"
#include "oscilloscope/oscilloscope.h"
#include "midi/midi.h"

// Create display object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Create input handler
Input input_handler;

// Create screen objects
OscilloscopeRoot oscilloscope_screen(&display);
MidiRoot midi_screen(&display);

// Current screen pointer
ScreenInterface* current_screen = &oscilloscope_screen;

// MIDI interface
MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, MIDI);

// NeoPixel setup
Adafruit_NeoPixel pixels(1, NEO_PIXEL_PIN, NEO_GRB + NEO_KHZ800);

// Encoder object
ESP32Encoder encoder;

// Global variables
uint32_t period = 0;

// Interrupt handler for sync input
void IRAM_ATTR sync_handle() {
    static uint32_t last_sync = 0;
    period = millis() - last_sync;
    last_sync = millis();
}

void setup() {
    // Initialize serial
    Serial.begin(115200);
    Serial2.begin(31250, SERIAL_8N1, 16, 17); // MIDI baud rate

    // Initialize display
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("SSD1306 allocation failed"));
        for(;;);
    }
    display.clearDisplay();
    display.display();

    // Initialize input handler
    input_handler = Input();

    // Initialize MIDI
    MIDI.begin(MIDI_CHANNEL_OMNI);
    MIDI.setHandleNoteOn(MidiRoot::handle_note_on);
    MIDI.setHandleNoteOff(MidiRoot::handle_note_off);
    MIDI.setHandleControlChange(MidiRoot::handle_control_change);

    // Initialize NeoPixel
    pixels.begin();
    pixels.setBrightness(50);
    pixels.setPixelColor(0, pixels.Color(0, 0, 255));
    pixels.show();

    // Initialize PWM
    ledcSetup(PWM_0, PWM_FREQ, PWM_RESOLUTION);
    ledcSetup(PWM_1, PWM_FREQ, PWM_RESOLUTION);
    ledcSetup(PWM_2, PWM_FREQ, PWM_RESOLUTION);
    ledcAttachPin(PWM_0_PIN, PWM_0);
    ledcAttachPin(PWM_1_PIN, PWM_1);
    ledcAttachPin(PWM_2_PIN, PWM_2);

    // Initialize encoder
    encoder.attachHalfQuad(ENCODER_A, ENCODER_B);
    encoder.setCount(0);

    // Configure pins
    pinMode(BUTTON_A, INPUT_PULLUP);
    pinMode(ENCODER_SW, INPUT_PULLUP);
    pinMode(ENCODER_A, INPUT_PULLUP);
    pinMode(ENCODER_B, INPUT_PULLUP);
    pinMode(SYNC_IN, INPUT);
    pinMode(SYNC_OUT, OUTPUT);

    // Configure sync interrupt
    attachInterrupt(SYNC_IN, sync_handle, RISING);

    // Set initial screen
    current_screen->enter();
}

void loop() {
    // Handle MIDI input
    MIDI.read();

    // Get input events
    Event event = input_handler.get_inputs();

    // Handle screen switching
    if (event.button_a == ButtonPress) {
        if (current_screen == &oscilloscope_screen) {
            current_screen->exit();
            current_screen = &midi_screen;
            current_screen->enter();
        } else {
            current_screen->exit();
            current_screen = &oscilloscope_screen;
            current_screen->enter();
        }
    }

    // Update current screen
    current_screen->update(&event);
} 
