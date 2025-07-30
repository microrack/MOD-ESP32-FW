# Code Style and Formatting Rules

## Overview

This document describes the code style used in the mod-esp32-fw project and the formatting rules configured in `.clang-format`.

## Basic Style Principles

### 1. Indentation and Formatting
- **Indentation**: 4 spaces (no tabs)
- **Maximum line length**: 120 characters
- **Brace style**: K&R (braces attached to keywords)

### 2. Naming Conventions

#### Classes and Structures
```cpp
// Classes: PascalCase
class MidiRoot : public ScreenInterface {
class OscilloscopeRoot : public ScreenInterface {
class ScreenSwitcher {
```

#### Variables and Functions
```cpp
// Variables: snake_case
int button_a_press_time;
uint32_t last_button_a_change_time;
bool button_a_pressed;

// Functions: snake_case
void get_inputs();
void set_screen(size_t index);
void update(Event* event);
```

#### Constants
```cpp
// Constants: UPPER_SNAKE_CASE
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
const int BUTTON_A = 38;
const uint32_t PWM_FREQ = 78125;
```

#### Enumerations
```cpp
// Enumerations: PascalCase for type and values
typedef enum {
    ButtonNone,
    ButtonPress,
    ButtonRelease,
    ButtonHold,
    ButtonSize
} Button;

enum class DisplayMode {
    Single,
    Joined,
    Split
};

enum MidiClkType {
    MidiClkInt,
    MidiClkExt,
};

enum MenuItems {
    MenuChannel,
    MenuOutA,
    MenuOutB,
    MenuOutC,
    MenuClock,
    MenuCount
};
```

### 3. File Structure

#### Header Files
- Use `#pragma once`
- Include necessary headers in logical order
- Type definitions and constants at the beginning of the file

```cpp
#pragma once

#include <stdint.h>
#include <Arduino.h>
#include "board.h"
#include "urack_types.h"

// Constants and type definitions
typedef enum {
    // ...
} Button;

// Class declarations
class Input {
    // ...
};
```

#### Implementation Files
- Include the corresponding header file first
- Constant definitions at the beginning of the file
- Class method implementations

```cpp
#include "input.h"

// Local constants
#define DEBOUNCE_TIME 50

// Method implementations
Input::Input() {
    // ...
}
```

### 4. Code Organization

#### Class Member Order
1. Public methods
2. Protected methods
3. Private methods
4. Private variables

#### Constructors
- Use initialization lists
- Simple initializations on one line
- Complex initializations - one per line

```cpp
ScreenSwitcher::ScreenSwitcher()
    : screens(nullptr), screen_count(0), current_index(0) {
}

ScreenSwitcher::ScreenSwitcher(ScreenInterface** screens, size_t screen_count)
    : screens(screens), screen_count(screen_count), current_index(0) {
}
```

### 5. Comments

#### Documentation Comments
- For public class methods
- Describe purpose and parameters

```cpp
// Get the current screen index
size_t get_current() const;

// Set the current screen to the given index
void set_screen(size_t index);
```

#### Inline Comments
- For complex logic
- Explain non-obvious decisions

```cpp
// Handle Button A with immediate events but debounce inhibition
if (button_a_reading != last_button_a_state && 
    (current_time - last_button_a_change_time) > DEBOUNCE_TIME) {
```

### 6. Error Handling

#### Condition Checks
- Simple checks at the beginning of functions
- Early return on errors

```cpp
void ScreenSwitcher::set_screen(size_t index) {
    if (screen_count == 0 || index >= screen_count) {
        return;
    }
    // ...
}
```

### 7. Arduino-Specific Rules

#### Initialization
- Use `pinMode()` for pin configuration
- Library initialization in `setup()`

```cpp
void setup() {
    Serial.begin(SERIAL_BAUDRATE);
    
    pinMode(BUTTON_A, INPUT_PULLUP);
    pinMode(ENCODER_SW, INPUT_PULLUP);
    
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("SSD1306 allocation failed"));
        for(;;);
    }
}
```

#### Main Loop
- Minimal logic in `loop()`
- Delegate processing to specialized classes

```cpp
void loop() {
    Event event = input_handler.get_inputs();
    screen_switcher.update(&event);
}
```

## Clang-format Settings

The `.clang-format` file is configured for automatic code formatting according to the described style. Main settings:

- **BasedOnStyle: Google** - based on Google style
- **IndentWidth: 4** - 4-space indentation
- **ColumnLimit: 120** - maximum line length of 120 characters
- **PointerAlignment: Left** - pointer asterisks attached to type
- **BreakBeforeBraces: Attach** - braces attached to keywords

## Usage Recommendations

1. **Automatic formatting**: Use clang-format for automatic code formatting
2. **Style checking**: Configure IDE for automatic style application on save
3. **Consistency**: Follow established rules to maintain code uniformity
4. **Readability**: Priority is given to code readability over brevity

## Exceptions

In some cases, deviations from the rules are acceptable:
- Compatibility with external libraries
- Performance optimization
- ESP32 platform-specific requirements 