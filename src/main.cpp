#include <Arduino.h>
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
const int PWM = 33;
const int DAC_1 = 25;
const int DAC_2 = 26;
const int ADC_0 = 36;
const int ADC_1 = 37;
const int SYNC_IN = 18;
const int SYNC_OUT = 19;

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
    ledcSetup(0, 16000, 12);
    ledcAttachPin(PWM, 0);
    ledcSetup(1, 16000, 12);
    ledcAttachPin(DAC_1, 1);
    ledcSetup(2, 16000, 12);
    ledcAttachPin(DAC_2, 2);

    // Initialize random seed
    randomSeed(analogRead(0));

    // Initialize serial communication
    Serial.begin(9600);

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

    // Update display every 20ms
    static uint32_t t_display = millis();
    if(millis() - t_display > 20) {
        display.clearDisplay();
        display.setCursor(0, 0);

        display.printf("%+.1f %+.1f",
            -5. + 10. * analogRead(ADC_0)/4096,
            -5. + 10. * analogRead(ADC_1)/4096
        );
        display.write('\n');
        draw_inputs(
            digitalRead(BUTTON_A) << 4 |
            digitalRead(ENCODER_SW) << 6
        );

        display.printf("  %d\n", encoder.getCount());
        display.printf("  note: %d", last_note);

        display.write('\n');
        if(period > 0) {
            display.printf("sync: %d ms, %d BPM", period, 30000/period);
        }

        display.display();
    }

    // Update DAC and NeoPixels every 10ms
    static uint32_t t_dac = millis();
    if(millis() - t_dac > 10) {
        ledcWrite(0, encoder.getCount() * 16);
        ledcWrite(1, encoder.getCount() * 16);
        ledcWrite(2, encoder.getCount() * 16);

        strip.setPixelColor(0, random(0, 255), random(0, 255), random(0, 255));
        strip.setPixelColor(1, random(0, 255), random(0, 255), random(0, 255));
        strip.setPixelColor(2, random(0, 255), random(0, 255), random(0, 255));
        strip.setPixelColor(3, random(0, 255), random(0, 255), random(0, 255));
        strip.show();

        uint8_t value = map(analogRead(ADC_0), 0, 4096, 0, 127);
        MIDI.sendNoteOn(value, 127, 1);
        delay(50);
        MIDI.sendNoteOff(value, 127, 1);

        t_dac = millis();
    }

    // Handle sync output
    static uint32_t t_sync = millis();
    if(millis() - t_sync > encoder.getCount() * 2) {
        digitalWrite(SYNC_OUT, HIGH);
        delay(1);
        digitalWrite(SYNC_OUT, LOW);
        t_sync = millis();
    }
    
    delay(2);
} 