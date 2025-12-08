# URack ESP32 Firmware

[![Build Status](https://github.com/microrack/MOD-ESP32-FW/actions/workflows/release.yml/badge.svg)](https://github.com/microrack/MOD-ESP32-FW/actions)
[![License](https://img.shields.io/github/license/microrack/MOD-ESP32-FW)](LICENSE)

Professional firmware for URack modular synthesizer module based on ESP32, featuring MIDI processing, real-time oscilloscope, audio synthesis, and CV/Gate generation.

## 🎹 Features

- **MIDI Processing**: Full MIDI I/O with configurable routing and note priority
- **Audio Synthesis**: Real-time audio generation using Mozzi library
- **CV/Gate Outputs**: 1V/octave pitch CV, velocity-controlled gates
- **Oscilloscope**: Dual-channel signal visualization with multiple display modes
- **OLED Display**: 128x64 high-contrast display with intuitive navigation
- **User Interface**: Rotary encoder and buttons for easy control
- **RGB LED**: Visual feedback and status indication
- **Persistent Settings**: Configuration saved to non-volatile storage
- **Web Flasher**: Browser-based firmware updates via WebUSB

### Building from Source

See [CONTRIBUTING.md](CONTRIBUTING.md) for detailed development guidelines.

```bash
# Clone and build
git clone https://github.com/microrack/MOD-ESP32-FW.git
cd MOD-ESP32-FW
pio run

# Flash to device
pio run --target upload

# View serial output
pio device monitor -b 115200
```

### Code Structure

- **Screen System**: Modular UI with `ScreenInterface` pattern
- **Event-Driven Input**: User interactions via `Event` structs
- **MIDI Processing**: `SignalProcessor` converts MIDI to CV/Audio
- **State Management**: Persistent settings in ESP32 NVS

For detailed architecture, see [ARCHITECTURE.md](ARCHITECTURE.md).

### Adding Features

Common extension points:
- **New Screen**: Implement `ScreenInterface`, add to screen array
- **MIDI Handler**: Add method to `SignalProcessor`
- **Audio Algorithm**: Register Mozzi callback with `SignalProcessor`

## 🚢 Releases & CI/CD

### Automatic Releases

When adding a tag in format `vX.X.X`, GitHub Actions automatically:
1. Builds firmware using PlatformIO
2. Creates GitHub Release with firmware files
3. Updates web interface for flashing

### Creating a Release

```bash
# Create and push tag (version is automatically extracted from tag)
git tag v1.0.0
git push origin v1.0.0
```

**Note:** Version is automatically extracted from tag name, so use format `vX.X.X`

### Release Artifacts

- `firmware.bin`: Flash file for ESP32
- `firmware.elf`: Debug symbols
- `firmware_release.zip`: Complete release package

## 🧪 Testing

### Test Mode

Built-in hardware test mode validates all I/O:
- Display functionality
- Encoder and button response  
- MIDI input/output
- CV/Gate outputs
- LED indicators

Activate via NVS flag (see `testmode.cpp`).

### Serial Debugging

Enable debug output in `board.h`:
```cpp
const bool DEBUG_MIDI_PROCESSOR = true;
```

Monitor at 115200 baud:
```bash
pio device monitor
```

## 📚 Documentation

- **[ARCHITECTURE.md](ARCHITECTURE.md)**: Comprehensive system architecture, data flow, and design patterns
- **[CONTRIBUTING.md](CONTRIBUTING.md)**: Development guidelines, code style, and contribution process
- **[.cursorrules](.cursorrules)**: AI assistant context for Cursor, Copilot, Claude Code, etc.
- **Inline Comments**: Hardware-specific details and complex algorithm explanations

## 🤖 AI Assistant Support

This project is optimized for AI coding assistants (GitHub Copilot, Cursor, Claude Code):

- **Comprehensive architecture documentation** in ARCHITECTURE.md
- **AI-specific guidelines** in .cursorrules
- **Clear code patterns** and naming conventions
- **Inline documentation** for complex hardware interactions
- **Context-rich comments** explaining embedded systems concepts

## 🔧 Technology Stack

- **Platform**: ESP32 (Xtensa LX6 dual-core @ 240MHz)
- **Framework**: Arduino for ESP32
- **Build System**: PlatformIO
- **Language**: C++11/14
- **Key Libraries**:
  - [Mozzi](https://sensorium.github.io/Mozzi/): Audio synthesis
  - [Sigscoper](https://github.com/microrack/Sigscoper): Oscilloscope
  - [Adafruit GFX/SSD1306](https://github.com/adafruit): Display
  - [ESP32Encoder](https://github.com/madhephaestus/ESP32Encoder): Rotary encoder

## 🤝 Contributing

Contributions are welcome! Please read [CONTRIBUTING.md](CONTRIBUTING.md) for:
- Development setup
- Code style guidelines
- Testing procedures
- Pull request process

## 📄 License

This project is licensed under the terms specified in the LICENSE file.

## 🙏 Acknowledgments

- URack platform: [microrack/urack-platform](https://github.com/microrack/urack-platform)
- Mozzi audio library: [sensorium/Mozzi](https://github.com/sensorium/Mozzi)
- Sigscoper library: [microrack/Sigscoper](https://github.com/microrack/Sigscoper)

## 📞 Support

- **Issues**: Report bugs or request features on [GitHub Issues](https://github.com/microrack/MOD-ESP32-FW/issues)
- **Discussions**: Ask questions in [GitHub Discussions](https://github.com/microrack/MOD-ESP32-FW/discussions)

## 🗺️ Roadmap

Planned features and improvements:
- Additional waveform generators
- Expanded MIDI CC mapping
- Custom LFO generators  
- Enhanced sequencer integration
- More oscilloscope trigger modes

---

**Made with ❤️ for the modular synthesis community**

## 🚀 Quick Start

### Hardware Requirements

- ESP32 DevKit or compatible board (dual-core Xtensa LX6)
- 128x64 OLED display (SSD1306, I2C)
- Rotary encoder with push button
- MIDI interface (optional, for MIDI I/O)
- USB cable for programming and power

### Software Requirements

- Python 3.7 or later
- PlatformIO CLI or IDE
- Web browser with WebUSB support (Chrome, Edge) for web flashing

## 📦 Installation

### Option 1: Web Flasher (Easiest)

Available at: `https://[username].github.io/[repository-name]/`

1. Open web page in browser with WebUSB support (Chrome, Edge)
2. Click "Connect Device"
3. Select your ESP32 device from the list
4. Click "Flash Device"

**Note:** WebUSB requires HTTPS connection, which is provided by GitHub Pages.

#### GitHub Pages Setup:

1. Go to repository settings (Settings)
2. In "Pages" section select source "GitHub Actions"
3. Make sure GitHub Actions are enabled in repository

### Option 2: Local Build

```bash
# Install PlatformIO
pip install platformio

# Clone repository
git clone https://github.com/microrack/MOD-ESP32-FW.git
cd MOD-ESP32-FW

# Build firmware
pio run

# Upload to device
pio run --target upload

# Monitor serial output
pio device monitor
```

## 📂 Project Structure

```
MOD-ESP32-FW/
├── src/                    # Firmware source code
│   ├── main.cpp           # Application entry point
│   ├── board.h            # Hardware pin definitions
│   ├── input/             # User input handling (encoder, buttons)
│   ├── midi/              # MIDI processing and configuration UI
│   ├── oscilloscope/      # Real-time signal visualization
│   ├── signal_processor/  # MIDI to CV/Gate/Audio conversion
│   └── osc/               # Audio synthesis (Mozzi integration)
├── web/                   # Web-based firmware flasher
├── scripts/               # Build and release automation
├── .github/workflows/     # CI/CD automation (GitHub Actions)
├── platformio.ini         # PlatformIO build configuration
├── nvs.csv               # Non-volatile storage partition table
├── ARCHITECTURE.md        # Detailed system architecture
├── CONTRIBUTING.md        # Contribution guidelines
├── .cursorrules          # AI assistant context and guidelines
└── README.md             # This file
```

## 🎛️ Hardware Configuration

### Pin Definitions (see board.h for details)

| Component | Pin(s) | Description |
|-----------|--------|-------------|
| OLED Display | SDA/SCL | I2C (address 0x3C) |
| Encoder | 34, 35, 39 | Rotary encoder with switch |
| Button A | 38 | Additional user button |
| MIDI | RX:16, TX:17 | UART (31250 baud) |
| Audio Out | 26, 25 | Dual PWM audio (Mozzi) |
| CV Out | 33 | PWM CV output (1V/octave) |
| Clock/Reset | 12, 13 | Gate/trigger outputs |
| ADC | 36, 37 | Analog inputs for oscilloscope |
| Sync | 18, 19 | External sync in/out |
| RGB LED | 23 | NeoPixel status indicator |

## 🎵 MIDI Features

- **Configurable MIDI channel** (1-16 or All)
- **Per-output channel mapping**: Route different MIDI channels to different CV outputs
- **Note priority modes**: Last, Low, High
- **Polyphonic voice allocation**: Track up to 8 simultaneous notes per channel
- **MIDI clock sync**: External tempo synchronization
- **Pitch bend**: ±2 semitone range (configurable)
- **CC to CV mapping**: Control voltages from continuous controllers
- **Aftertouch support**: Channel pressure to CV

## 🔊 Audio & CV Outputs

### Output Channels

1. **Channel A & B** (Mozzi): Dual-channel audio synthesis (PWM, 32768 Hz)
2. **Channel C** (PWM): 1V/octave pitch CV with velocity control
3. **Clock/Reset** (GPIO): Digital gate/trigger outputs for sequencing

### CV Scaling

- **Pitch**: 1V/octave standard (Middle C = 60)
- **Gate**: Velocity-scaled (0-5V)
- **PWM Resolution**: 10-bit (1024 steps)
- **PWM Frequency**: 32768 Hz

## 📊 Oscilloscope

- **Dual-channel display**: Simultaneously monitor two signals
- **Display modes**: Single, Joined, Split
- **Time scales**: 0.25ms to 500ms per division (11 steps)
- **Trigger modes**: Auto, Normal, Rolling
- **Crosshair navigation**: Inspect waveform details

## 🛠️ Development

### Building from Source 