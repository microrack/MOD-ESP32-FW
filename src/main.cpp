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
#include "screen_switcher.h"

// Create display object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Create input handler
Input input_handler;

// Create screen objects
OscilloscopeRoot oscilloscope_screen(&display);
MidiRoot midi_screen(&display);

// Create screen array and switcher
ScreenInterface* screens[] = {&midi_screen, &oscilloscope_screen};
const size_t screen_count = sizeof(screens) / sizeof(screens[0]);
ScreenSwitcher screen_switcher(screens, screen_count);

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

    screen_switcher.set_screen(0);
}

void loop() {
    // Get input events
    Event event = input_handler.get_inputs();

    // Handle screen switching
    if (event.button_a == ButtonPress) {
        screen_switcher.set_screen(screen_switcher.get_next());
    }

    // Update current screen
    screen_switcher.update(&event);

    Event::print(event);
} 
