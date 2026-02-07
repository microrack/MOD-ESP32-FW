# MICRORACK ESP32 Module Firmware

Firmware for [MICRORACK module](https://microrack.org/market/products/mod-esp32) based on ESP32 with MIDI, oscilloscope and other features support.

## Automatic Releases

When adding a tag in format `vX.X.X`, automatically:
1. Firmware is built using PlatformIO
2. GitHub Release is created with firmware files
3. Web interface for flashing is updated

### How to create a release:

```bash
# Create and push tag (version is automatically taken from tag)
git tag v1.0.0
git push origin v1.0.0
```

**Note:** Version is automatically extracted from tag name, so use format `vX.X.X`

## Web Interface for Flashing

Available at: `https://[username].github.io/[repository-name]/`

### GitHub Pages Setup:

1. Go to repository settings (Settings)
2. In "Pages" section select source "GitHub Actions"
3. Make sure GitHub Actions are enabled in repository

### Using the web interface:

1. Open web page in browser with WebUSB support (Chrome, Edge)
2. Click "Connect Device"
3. Select your ESP32 device from the list
4. Click "Flash Device"

**Note:** WebUSB requires HTTPS connection, which is provided by GitHub Pages.

## Local Build

```bash
# Install PlatformIO
pip install platformio

# Build firmware
pio run

# Upload to device
pio run --target upload

# Monitor port
pio device monitor
```

## Project Structure

- `src/` - firmware source code
- `web/` - web interface for flashing
- `.github/workflows/` - GitHub Actions for automation
- `platformio.ini` - PlatformIO configuration

## Supported Features

- MIDI processing
- Oscilloscope
- OLED display
- RGB LED indication
- Control encoder

## Requirements

- ESP32 DevKit or compatible board
- PlatformIO IDE or CLI
- Browser with WebUSB support for web interface 
