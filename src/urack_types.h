/**
 * URack Common Types and Interfaces
 * 
 * Defines core abstractions used throughout the firmware:
 * - Display type alias
 * - ScreenInterface for modular UI screens
 * 
 * The screen system enables a clean separation of concerns where each
 * UI mode (oscilloscope, MIDI settings, etc.) implements the same interface.
 */

#pragma once

#include <stdint.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include "input/input.h"

// Type alias for display to allow easy substitution if hardware changes
typedef Adafruit_SSD1306 Display;

/**
 * ScreenInterface: Base class for all UI screens
 * 
 * Implements a lifecycle pattern for screen management:
 * 
 * Lifecycle:
 *   1. enter() - Called once when screen becomes active
 *   2. update() - Called repeatedly while screen is active (processes input, redraws)
 *   3. exit() - Called once when leaving the screen
 * 
 * Usage example:
 *   class MyScreen : public ScreenInterface {
 *       void enter() override { // Initialize screen state }
 *       void exit() override { // Cleanup resources }
 *       void update(Event* event) override {
 *           // Handle encoder rotation, button presses
 *           // Redraw display based on state
 *       }
 *   };
 */
class ScreenInterface {
public:
    ScreenInterface(Display* display) : display(display) {}
    virtual ~ScreenInterface() = default;

    /**
     * Called when the screen becomes active
     * Use for initialization, clearing display, showing initial state
     */
    virtual void enter() = 0;

    /**
     * Called when exiting the screen
     * Use for cleanup, saving state, or resetting hardware
     */
    virtual void exit() = 0;

    /**
     * Called every frame while screen is active
     * @param event User input events (encoder rotation, button presses)
     * 
     * Typical pattern:
     * 1. Process input (event->encoder, event->button_*)
     * 2. Update internal state
     * 3. Redraw display (display->clearDisplay(), draw, display->display())
     */
    virtual void update(Event* event) = 0;

protected:
    Display* display;  // Shared 128x64 OLED display instance
};
