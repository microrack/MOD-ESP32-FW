# Contributing to MOD-ESP32-FW

Thank you for your interest in contributing to the URack ESP32 Firmware project! This document provides guidelines and information for contributors.

## Table of Contents
- [Getting Started](#getting-started)
- [Development Setup](#development-setup)
- [Code Style](#code-style)
- [Testing](#testing)
- [Submitting Changes](#submitting-changes)
- [Architecture Overview](#architecture-overview)

## Getting Started

### Prerequisites
- **Hardware**: ESP32 DevKit or compatible board
- **Software**: 
  - Python 3.7+
  - PlatformIO CLI or IDE
  - Git
- **Optional**: MIDI interface, oscilloscope for testing CV outputs

### Development Setup

1. **Clone the repository**
   ```bash
   git clone https://github.com/microrack/MOD-ESP32-FW.git
   cd MOD-ESP32-FW
   ```

2. **Install PlatformIO**
   ```bash
   pip install platformio
   ```

3. **Build the firmware**
   ```bash
   pio run
   ```

4. **Upload to device**
   ```bash
   pio run --target upload
   ```

5. **Monitor serial output**
   ```bash
   pio device monitor
   ```

### Project Structure

```
MOD-ESP32-FW/
├── src/                    # Source code
│   ├── main.cpp           # Entry point
│   ├── board.h            # Pin definitions and hardware config
│   ├── input/             # User input handling
│   ├── midi/              # MIDI processing and UI
│   ├── oscilloscope/      # Oscilloscope display
│   ├── signal_processor/  # MIDI to CV/audio conversion
│   └── osc/               # Audio synthesis
├── web/                   # Web-based flasher
├── scripts/               # Build scripts
├── platformio.ini         # PlatformIO configuration
├── ARCHITECTURE.md        # Detailed architecture documentation
└── README.md             # Basic project information
```

## Code Style

### General Guidelines

- **Language**: C++11/14
- **Indentation**: 4 spaces (no tabs)
- **Line length**: Prefer 100 characters or less
- **Headers**: Use `#pragma once` instead of include guards

### Naming Conventions

```cpp
// Classes: PascalCase
class SignalProcessor { };

// Functions and methods: snake_case
void handle_note_on(uint8_t note);

// Constants: UPPER_SNAKE_CASE
const int SCREEN_WIDTH = 128;
#define MIDI_BAUDRATE 31250

// Enums: PascalCase with prefixed values
enum OutChannelType {
    OutTypeMozzi,
    OutTypePwm,
    OutTypeGpio
};

// Variables: snake_case
int current_note;
MidiSettingsState* settings_state;
```

### Code Organization

```cpp
// Order of includes
#include <Arduino.h>        // System headers
#include <Wire.h>

#include <Adafruit_GFX.h>  // Library headers

#include "board.h"          // Local headers
#include "input/input.h"

// Class declaration order
class MyClass {
public:                     // Public interface first
    MyClass();
    void public_method();
    
private:                    // Private implementation
    void private_method();
    int member_variable;
    static const int CONSTANT = 42;
};
```

## Testing

### Manual Testing

Before submitting changes:

1. **Build verification**
   ```bash
   pio run
   ```

2. **Hardware testing**
   - Test on actual ESP32 hardware
   - Verify OLED display updates
   - Check encoder and button response
   - Test MIDI input/output if modified
   - Verify CV outputs with oscilloscope (if applicable)

3. **Test mode**
   - Use built-in test mode for comprehensive hardware validation
   - Activated via NVS flag (see testmode.cpp)

### Serial Debugging

Enable debug output:
```cpp
// In board.h
const bool DEBUG_MIDI_PROCESSOR = true;
```

Monitor output:
```bash
pio device monitor -b 115200
```

### Testing Checklist

- [ ] Code compiles without warnings
- [ ] Firmware uploads and runs on ESP32
- [ ] Display updates correctly
- [ ] User input (encoder, buttons) works
- [ ] MIDI functionality intact (if applicable)
- [ ] No memory leaks or crashes
- [ ] Serial debug output is clean

## Submitting Changes

### Pull Request Process

1. **Fork the repository** and create a feature branch
   ```bash
   git checkout -b feature/my-new-feature
   ```

2. **Make your changes**
   - Follow code style guidelines
   - Add comments for complex logic
   - Update documentation if needed

3. **Test thoroughly**
   - Build and test on hardware
   - Verify no regressions

4. **Commit with clear messages**
   ```bash
   git commit -m "Add feature: brief description
   
   Detailed explanation of changes and why they were made.
   Fixes #issue_number"
   ```

5. **Push to your fork**
   ```bash
   git push origin feature/my-new-feature
   ```

6. **Create Pull Request**
   - Provide clear description of changes
   - Reference any related issues
   - Include test results

### Commit Message Format

```
Short summary (50 chars or less)

Detailed explanation of the change, why it was needed,
and any relevant context. Wrap at 72 characters.

- Bullet points for multiple changes
- Reference issues with #123

Fixes #123
```

## Architecture Overview

For detailed architecture information, see [ARCHITECTURE.md](ARCHITECTURE.md). Key points:

### Screen System
All UI screens implement `ScreenInterface`:
- `enter()`: Initialize when screen becomes active
- `exit()`: Clean up when leaving screen
- `update(Event*)`: Handle user input and redraw

### MIDI Processing
- MIDI messages → `SignalProcessor` → CV/Gate outputs
- Persistent settings in ESP32 NVS
- Polyphonic voice allocation via `NoteHistory`

### Signal Processing
- Converts MIDI to control voltages (1V/octave)
- Supports multiple output types (Mozzi audio, PWM, GPIO)
- Real-time processing with Mozzi library

## Common Development Tasks

### Adding a New Screen

1. Create class implementing `ScreenInterface`
   ```cpp
   class MyScreen : public ScreenInterface {
   public:
       MyScreen(Display* display);
       void enter() override;
       void exit() override;
       void update(Event* event) override;
   private:
       Display* display;
   };
   ```

2. Add to screen array in `main.cpp`
   ```cpp
   MyScreen my_screen(&display);
   ScreenInterface* screens[] = {
       &oscilloscope_screen,
       &midi_screen,
       &my_screen  // Add here
   };
   ```

### Adding a MIDI Feature

1. Add handler in `SignalProcessor`
   ```cpp
   void SignalProcessor::handle_my_midi_event(uint8_t value) {
       // Process MIDI event
       // Update outputs
   }
   ```

2. Wire up in MIDI library callback
3. Update `MidiInfo` display if needed

### Modifying Hardware Output

1. Update constants in `board.h` if needed
2. Modify output functions in `SignalProcessor`
   ```cpp
   void SignalProcessor::out_pitch(int channel, int note) {
       int pwm_value = calculate_cv_value(note);
       analogWrite(OUT_CHANNELS[channel].pin, pwm_value);
   }
   ```

3. Document scaling and calibration

## Best Practices

### Memory Management
- Prefer stack allocation for small objects
- Minimize heap allocations (embedded constraints)
- Use `const` and `constexpr` where possible
- Be mindful of buffer sizes

### Real-Time Code
- Audio callbacks must be fast and deterministic
- No blocking operations in ISRs or audio loops
- Minimize floating-point in critical paths
- Use integer math where possible

### Hardware Interaction
- Always use named constants for pins
- Document timing requirements
- Add bounds checking for ADC/PWM values
- Consider debouncing for inputs

### Documentation
- Comment complex algorithms
- Explain hardware-specific magic numbers
- Document units (ms, Hz, volts, MIDI values)
- Update ARCHITECTURE.md for significant changes

## Resources

- [PlatformIO Documentation](https://docs.platformio.org/)
- [ESP32 Arduino Core](https://github.com/espressif/arduino-esp32)
- [Mozzi Library](https://sensorium.github.io/Mozzi/)
- [MIDI Specification](https://www.midi.org/specifications)
- [Modular Synthesis Basics](https://learningmodular.com/)

## Getting Help

- **Issues**: Report bugs or request features on GitHub Issues
- **Discussions**: Ask questions in GitHub Discussions
- **Documentation**: See ARCHITECTURE.md and inline code comments

## License

By contributing, you agree that your contributions will be licensed under the same license as the project (see LICENSE file).

## Recognition

Contributors will be acknowledged in release notes and the project README. Thank you for helping improve MOD-ESP32-FW!
