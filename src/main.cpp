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
ScreenInterface* screens[] = {&oscilloscope_screen, &midi_screen};
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
    display.setRotation(2);
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

    Event::print(event);
} 
