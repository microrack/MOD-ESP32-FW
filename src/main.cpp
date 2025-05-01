#include <Arduino.h>
#include <cstdint>
#include <driver/dac.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Encoder.h>
#include <Adafruit_NeoPixel.h>
#include <MIDI.h>

// Create a 'MIDI' object using MySettings bound to Serial2.
MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial2, MIDI, midi::DefaultSerialSettings);

// OLED display configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Pin definitions
const int BUTTON_A = 38;
const int ENCODER_SW = 39;
const int PWM_0_PIN = 33;
const int PWM_1_PIN = 25;
const int PWM_2_PIN = 26;
const int ADC_0 = 36;
const int ADC_1 = 37;
const int SYNC_IN = 18;
const int SYNC_OUT = 19;

const int PWM_0 = 0;
const int PWM_1 = 1;
const int PWM_2 = 2;

// PWM parameters
const uint32_t PWM_FREQ = 78125;
const uint8_t  PWM_RESOLUTION = 10;
const uint32_t PWM_MAX_VAL = (1 << PWM_RESOLUTION) - 1;

// NeoPixel configuration
Adafruit_NeoPixel strip = Adafruit_NeoPixel(4, 23, NEO_GRB + NEO_KHZ800);

// Encoder object
ESP32Encoder encoder;

// Global variables
uint32_t period = 0;
uint8_t last_note = 0;

// Interrupt handler for sync input
void IRAM_ATTR sync_handle() {
    static uint32_t last_sync = 0;
    period = millis() - last_sync;
    last_sync = millis();
}

// MIDI note on handler
void handle_note_on(byte channel, byte note, byte velocity) {
    last_note = note;
}

// Helper function to draw input states
void draw_inputs(uint8_t state) {
    for(uint8_t i = 0; i < 8; i++) {
        if(state & 1 << i) {
            display.write('+');
        } else {
            display.write('-');
        }
    }
}

void setup() {
    // Initialize NeoPixel strip
    strip.begin();
    strip.setBrightness(255);
    strip.clear();

    // Initialize encoder
    encoder.attachHalfQuad(34, 35);
    encoder.setCount(0);

    // Configure pins
    pinMode(BUTTON_A, INPUT_PULLUP);
    pinMode(ENCODER_SW, INPUT_PULLUP);
    pinMode(34, INPUT_PULLUP);
    pinMode(35, INPUT_PULLUP);
    pinMode(SYNC_IN, INPUT);
    pinMode(SYNC_OUT, OUTPUT);

    // Configure PWM channels
    ledcSetup(PWM_0, PWM_FREQ, PWM_RESOLUTION);
    ledcAttachPin(PWM_0_PIN, PWM_0);
    ledcSetup(PWM_1, PWM_FREQ, PWM_RESOLUTION);
    ledcAttachPin(PWM_1_PIN, PWM_1);
    ledcSetup(PWM_2, PWM_FREQ, PWM_RESOLUTION);
    ledcAttachPin(PWM_2_PIN, PWM_2);

    // Initialize random seed
    randomSeed(analogRead(0));

    // Initialize serial communication
    Serial.begin(115200);

    // Initialize OLED display
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("SSD1306 allocation failed"));
        for(;;); // Don't proceed, loop forever
    }

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.cp437(true);
    display.setRotation(2);

    // Configure sync interrupt
    attachInterrupt(SYNC_IN, sync_handle, RISING);

    // Initialize MIDI
    MIDI.begin(MIDI_CHANNEL_OMNI);
    MIDI.setHandleNoteOn(handle_note_on);
    MIDI.turnThruOff();
}

void loop() {
    MIDI.read();

    ledcWrite(PWM_0, PWM_MAX_VAL);
    ledcWrite(PWM_1, PWM_MAX_VAL);
    ledcWrite(PWM_2, PWM_MAX_VAL);
    delay(100);
    ledcWrite(PWM_0, PWM_MAX_VAL / 2);
    ledcWrite(PWM_1, PWM_MAX_VAL / 2);
    ledcWrite(PWM_2, PWM_MAX_VAL / 2);
    delay(1000);
} 
