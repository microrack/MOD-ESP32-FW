#include <Arduino.h>
#include <cstdint>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>
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
ScreenInterface* current_screen = &midi_screen;

// NeoPixel setup
Adafruit_NeoPixel pixels(1, NEO_PIXEL_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
    // Initialize serial
    Serial.begin(115200);

    // Initialize display
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("SSD1306 allocation failed"));
        for(;;);
    }
    display.clearDisplay();
    display.display();

    // Initialize input handler
    input_handler = Input();

    // Initialize NeoPixel
    pixels.begin();
    pixels.setBrightness(50);
    pixels.setPixelColor(0, pixels.Color(0, 0, 255));
    pixels.show();
    
    // Set initial screen
    current_screen->enter();
}

void loop() {
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

    Event::print(event);
} 
