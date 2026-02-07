# Architecture Documentation

## Project Overview

MOD-ESP32-FW is the firmware for the URack modular synthesizer module based on ESP32. It provides MIDI processing, oscilloscope visualization, and real-time audio signal generation capabilities for modular synthesis applications.

## System Architecture

### Hardware Platform
- **Microcontroller**: ESP32 DevKit (dual-core Xtensa LX6)
- **Display**: 128x64 OLED (SSD1306, I2C address 0x3C)
- **User Interface**: 
  - Rotary encoder with push button (pins 34, 35, 39)
  - Button A (pin 38)
  - RGB LED (NeoPixel, pin 23)
- **I/O**:
  - MIDI In/Out (UART pins 16/17, 31250 baud)
  - Analog inputs: ADC_0 (pin 36), ADC_1 (pin 37)
  - Audio outputs: 2 channels via Mozzi library (pins 26, 25)
  - PWM output (pin 33)
  - Sync In/Out (pins 18/19)
  - Clock/Reset outputs (pins 12/13)

### Software Architecture

```
┌─────────────────────────────────────────────────────────┐
│                       main.cpp                          │
│                    (Entry Point)                        │
└────────────┬────────────────────────────────────────────┘
             │
    ┌────────┴────────┐
    │                 │
    ▼                 ▼
┌─────────┐      ┌──────────────┐
│  Input  │      │   Display    │
│ Handler │      │ (SSD1306)    │
└────┬────┘      └──────┬───────┘
     │                  │
     │         ┌────────┴────────┐
     │         │                 │
     │         ▼                 ▼
     │   ┌──────────┐      ┌──────────┐
     └──>│   MIDI   │      │Oscillosc-│
         │  Screen  │      │   ope    │
         └────┬─────┘      └────┬─────┘
              │                 │
         ┌────┴────┐            │
         │         │            │
         ▼         ▼            │
    ┌────────┐ ┌──────────┐    │
    │  MIDI  │ │  MIDI    │    │
    │  Info  │ │ Settings │    │
    └────────┘ └────┬─────┘    │
                    │           │
              ┌─────┴──────┐    │
              │            │    │
              ▼            ▼    ▼
        ┌──────────────────────────┐
        │   SignalProcessor        │
        │  (MIDI → CV/Gate/Audio)  │
        └────────┬─────────────────┘
                 │
          ┌──────┴──────┐
          │             │
          ▼             ▼
     ┌────────┐    ┌──────────┐
     │ Mozzi  │    │   PWM    │
     │ Audio  │    │  Outputs │
     └────────┘    └──────────┘
```

## Core Components

### 1. Screen System

**Location**: `src/screen_switcher.{h,cpp}`

The firmware uses a screen-based UI architecture:
- `ScreenInterface`: Base interface for all screens
- `ScreenSwitcher`: Manages navigation between screens
- Main screens: Oscilloscope and MIDI

**Key Classes**:
- `Event`: User input events (encoder, buttons)
- `ScreenInterface`: Virtual interface with `enter()`, `exit()`, `update(Event*)`

### 2. Input Handling

**Location**: `src/input/`

Manages all user input:
- Rotary encoder with debouncing
- Two buttons (A and encoder switch)
- Long press detection (hold events)

**Key Classes**:
- `Input`: Polls hardware and generates Event structs
- `Event`: Contains encoder delta, button states, and timing

### 3. MIDI Processing

**Location**: `src/midi/`

Implements MIDI receive/transmit and routing:
- **MidiRoot**: Top-level MIDI screen manager
- **MidiInfo**: Displays incoming MIDI messages
- **MidiSettings**: Configurable MIDI parameters
- **MidiSettingsState**: Persistent MIDI configuration (stored in NVS)
- **NoteHistory**: Tracks polyphonic note on/off for proper voice allocation

**MIDI Features**:
- Configurable MIDI channel per output
- Note priority modes (last, low, high)
- CC to output mapping
- Aftertouch and pitch bend support
- MIDI clock sync

### 4. Signal Processing

**Location**: `src/signal_processor/`

Converts MIDI messages to control voltages and audio:

**Output Modes**:
1. **Mozzi Audio** (Channels A & B): Real-time audio synthesis
2. **PWM** (Channel C): 1V/octave CV for pitch
3. **GPIO** (Clock/Reset): Digital gate signals

**SignalProcessor** handles:
- Note → Pitch CV conversion (1V/octave standard)
- Velocity → Gate voltage
- CC → CV mapping
- Pitch bend with configurable range
- MIDI clock → trigger pulses
- Polyphonic voice allocation via NoteHistory

**Key Constants**:
- `PWM_NOTE_SCALE`: Converts MIDI notes to PWM values
- `MIDDLE_NOTE`: 60 (C4 reference)
- `PITCHBEND_RANGE_SEMITONES`: ±2 semitones

### 5. Oscilloscope

**Location**: `src/oscilloscope/`

Real-time signal visualization using the Sigscoper library:
- Dual-channel display (ADC_0, ADC_1)
- Multiple display modes: Single, Joined, Split
- Configurable time scales (0.25ms to 500ms per division)
- Rolling and triggered display modes
- Crosshair navigation for signal inspection

### 6. Audio Synthesis

**Location**: `src/osc/`

Uses the Mozzi library for audio generation:
- Dual-channel PWM audio output
- Configurable control rate: 1024 Hz
- Audio rate: 32768 Hz (PWM_FREQ)
- Integration with SignalProcessor for MIDI-driven synthesis

### 7. Test Mode

**Location**: `src/testmode.{h,cpp}`

Factory test mode for hardware validation:
- Activated via NVS flag
- Tests all I/O: buttons, encoder, LEDs, outputs
- Self-clearing after successful test completion

## Data Flow

### MIDI Input → Audio Output
```
MIDI UART RX (pin 16)
  ↓
MIDI Library parsing
  ↓
SignalProcessor::handle_note_on/off/cc/etc
  ↓
NoteHistory (voice allocation)
  ↓
out_pitch() → PWM CV
out_gate() → GPIO/PWM gate
  ↓
Hardware outputs (pins 26, 25, 33, etc.)
```

### User Input → Screen Updates
```
Hardware (encoder, buttons)
  ↓
Input::get_inputs()
  ↓
Event struct
  ↓
ScreenSwitcher::update(Event*)
  ↓
Active Screen::update(Event*)
  ↓
Display::display()
```

## Key Design Patterns

### 1. Interface-Based Screens
All screens implement `ScreenInterface` for consistent lifecycle management (enter/exit/update).

### 2. Event-Driven Updates
User inputs generate `Event` structs passed to active screens, enabling modal UI behavior.

### 3. State Management
- **MidiSettingsState**: Persistent settings in ESP32 NVS (Non-Volatile Storage)
- **SignalProcessor**: Runtime state (active notes, CV outputs)

### 4. Callback Architecture
SignalProcessor uses callbacks for:
- `update_audio_callback`: Mozzi audio generation
- `event_callback`: External MIDI event notification

## Memory Management

- **Stack-based objects**: Most objects created on stack in `setup()`
- **NVS**: Persistent storage for MIDI settings, test mode flag
- **EEPROM**: 64 bytes allocated (MIDI settings)
- **Buffers**: Signal buffers (128 samples × 2 channels) for oscilloscope

## Threading Model

- **Main loop**: Arduino loop() for UI and display updates
- **MIDI task**: FreeRTOS task for MIDI processing (created in SignalProcessor)
- **Mozzi**: Timer-driven audio callbacks (control and audio rate)

## Build System

**Platform**: PlatformIO
- **Framework**: Arduino for ESP32
- **Custom platform**: URack ESP32 platform (microrack/urack-platform)
- **Board**: mod-esp32-v1

**Key Dependencies**:
- `microrack/Sigscoper`: Oscilloscope functionality
- `Mozzi`: Audio synthesis library
- Adafruit libraries: GFX, SSD1306, NeoPixel
- ESP32Encoder: Rotary encoder handling

## Configuration Files

- **platformio.ini**: Build configuration, dependencies, board definition
- **nvs.csv**: Non-volatile storage partition layout
- **board.h**: Pin definitions and hardware constants

## Important Constants & Defines

```cpp
// Display
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCREEN_ADDRESS 0x3C

// Audio
#define PWM_FREQ 32768
#define PWM_RESOLUTION 10
#define MOZZI_CONTROL_RATE 1024

// MIDI
const unsigned long MIDI_BAUDRATE = 31250;
const int MIDI_CHANNEL_COUNT = 16;

// Timing
const int CLOCK_TICKS_PER_BEAT = 24  // MIDI clock standard
```

## Extension Points

To add new features:

1. **New Screen**: Implement `ScreenInterface`, add to screen array in main.cpp
2. **New MIDI Handler**: Add handler in SignalProcessor, update event_callback
3. **New Output Mode**: Extend OutChannelType enum, add handling in SignalProcessor
4. **New Audio Algorithm**: Implement Mozzi audio callback, register with SignalProcessor

## Common Development Tasks

### Adding a MIDI Feature
1. Add handler method in `SignalProcessor`
2. Update `ProcessorEventType` enum if needed
3. Wire handler in MIDI library callback
4. Update MidiInfo display if needed

### Adding a Display Screen
1. Create class implementing `ScreenInterface`
2. Add to screens array in `main.cpp`
3. Implement enter/exit/update methods
4. Handle encoder and button events

### Modifying CV Output
1. Adjust constants in `SignalProcessor` (PWM_NOTE_SCALE, etc.)
2. Update calibration in `out_pitch()` or `out_gate()`
3. Test with external oscilloscope/multimeter

## Debugging

- **Serial output**: 115200 baud on USB
- **Debug flags**: `DEBUG_MIDI_PROCESSOR` in board.h
- **Test mode**: Set NVS testmode flag to enter hardware test

## CI/CD

**GitHub Actions** (`.github/workflows/release.yml`):
- Triggered on version tags (v*.*.*)
- Builds firmware with PlatformIO
- Creates GitHub release with binaries
- Deploys web flasher to GitHub Pages

## References

- [Mozzi Library](https://sensorium.github.io/Mozzi/)
- [URack Platform](https://github.com/microrack/urack-platform)
- [PlatformIO Documentation](https://docs.platformio.org/)
- [ESP32 Documentation](https://docs.espressif.com/projects/esp-idf/)
