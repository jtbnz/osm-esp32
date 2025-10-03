# ESP32 Hybrid Crypto Miner

Dual-mode cryptocurrency miner for ESP32-S3 with 7-inch touchscreen display.

## Features

- **Bitcoin Mode**: SHA-256 lottery mining (~40-50 KH/s)
- **Duino-Coin Mode**: DUCO-S1 practical mining (~20-40 KH/s) with actual earnings
- 7-inch 800x480 RGB display with LVGL UI
- Capacitive touch interface
- Web configuration portal
- WiFi with AP fallback mode
- Real-time statistics and historical charts
- Mode switching via touch or web interface

## Hardware Requirements

- ESP32-S3-N16R8 (16MB Flash, 8MB PSRAM)
- VIEWESMART UEDX80480070ESP32 7-inch Touch Display
- 800x480 RGB display
- GT911 capacitive touch controller
- USB-C cable for flashing

## Quick Start - First Build

Your device is connected to `/dev/tty.usbserial-213220`.

### Using VS Code ESP-IDF Extension:

1. **Set Target** (first time only):
   - Command Palette (Cmd+Shift+P)
   - Type: **ESP-IDF: Set Espressif Device Target**
   - Select: **esp32s3**

2. **Build Project**:
   - Click ESP-IDF icon in status bar (bottom)
   - Click **Build** (hammer icon)
   - Or: **ESP-IDF: Build Project** from Command Palette

3. **Flash**:
   - Click **Flash** (lightning icon)
   - Or: **ESP-IDF: Flash Project**

4. **Monitor**:
   - Click **Monitor** (screen icon)
   - Or: **ESP-IDF: Monitor Device**

Expected serial output:
```
ESP32 Hybrid Crypto Miner v1.0.0
Bitcoin + Duino-Coin Dual Mining
NVS initialized
System running...
```

## Configuration

Edit `config.h` with your credentials:
```bash
cp config.h.example config.h
code config.h  # Edit with VS Code
```

Fill in:
- WiFi SSID/password
- Bitcoin wallet (optional for now)
- Duino-Coin username (optional for now)

## Project Status

✅ **Phase 1: Foundation COMPLETE**
- Project structure
- Build system
- Basic initialization

🚧 **Next**: Phase 2-13 (Configuration, WiFi, Display, Mining, etc.)

See [IMPLEMENTATION_PLAN.md](IMPLEMENTATION_PLAN.md) for full roadmap.

## Documentation

- [PREREQUISITES.md](PREREQUISITES.md) - Account setup
- [IMPLEMENTATION_PLAN.md](IMPLEMENTATION_PLAN.md) - Development plan
- [CLAUDE.md](CLAUDE.md) - Architecture notes

## License

MIT
