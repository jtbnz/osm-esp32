# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

ESP32-S3 hybrid cryptocurrency miner with 7-inch touchscreen display. Dual-mode mining combining Bitcoin (SHA-256 lottery mining) and Duino-Coin (DUCO-S1 practical mining). Educational/learning project for understanding different cryptocurrency mining approaches on embedded systems.

**Hardware**: VIEWESMART UEDX80480070ESP32 (ESP32-S3-N16R8, 16MB Flash, 8MB PSRAM, 800x480 RGB display, GT911 touch controller)

**Mining Modes**:
- **Bitcoin Mode**: SHA-256 hardware-accelerated, ~40-50 KH/s, Stratum protocol, lottery mining (extremely low probability but educational)
- **Duino-Coin Mode**: DUCO-S1 algorithm (SHA-1 based), ~20-40 KH/s, actual coin earnings within days (practical learning experience)

## Build System

This project uses **ESP-IDF** (native framework, not Arduino).

### Common Commands

```bash
# Set up ESP-IDF environment (run in each terminal session)
. $HOME/esp/esp-idf/export.sh

# Configure project
idf.py menuconfig

# Build firmware
idf.py build

# Flash to device via serial
idf.py -p /dev/ttyUSB0 flash

# Monitor serial output
idf.py -p /dev/ttyUSB0 monitor

# Build, flash, and monitor in one command
idf.py -p /dev/ttyUSB0 flash monitor

# Clean build
idf.py fullclean

# Check component dependencies
idf.py reconfigure
```

## Architecture

### Core Components

1. **Mining Engines** (Dual-mode)
   - **Bitcoin Mining** (`components/mining_bitcoin/`)
     - SHA-256 double hashing with ESP32 hardware acceleration
     - Stratum protocol client for solo pool connection
     - Block header construction and nonce iteration
     - Expected performance: 40-50 KH/s
     - Based on NerdMiner V2 architecture

   - **Duino-Coin Mining** (`components/mining_duinocoin/`)
     - DUCO-S1 algorithm (SHA-1 based with Kolka system)
     - Text-based protocol client for DUCO servers
     - Job request/response handling
     - Earnings tracking and accumulation
     - Expected performance: 20-40 KH/s

   - **Mining Common** (`components/mining_common/`)
     - Shared utilities for both engines
     - Hashrate calculation (moving average)
     - Temperature monitoring and throttling
     - Common networking functions

2. **Display System** (`components/display/`)
   - RGB interface driver (800x480, 16-bit color)
   - LVGL integration for UI rendering
   - GT911 capacitive touch handling (I2C)
   - Backlight management with 60-second timeout
   - Historical charts for hashrate/stats
   - Mode selector UI (Bitcoin / Duino-Coin toggle)
   - Mode-specific stat displays

3. **WiFi Manager** (`components/wifi_manager/`)
   - Station mode for normal operation
   - AP fallback mode for initial configuration (SSID: "ESP32-Miner-Setup")
   - SSID/password entry via web interface
   - Connection status monitoring
   - Automatic reconnection logic

4. **Web Server** (`components/webserver/`)
   - Dual-mode configuration interface (Bitcoin + Duino-Coin)
   - REST API for device control and stats
   - Mode switching endpoint
   - Serves static HTML/CSS/JS for config UI
   - Real-time status updates

5. **Configuration** (`components/config/`)
   - NVS (Non-Volatile Storage) for persistent settings
   - WiFi credentials (SSID, password)
   - Bitcoin config: pool URL, wallet address, worker name
   - Duino-Coin config: username, mining key, server endpoint
   - Active mode preference (persists across reboots)

6. **Statistics Tracker** (`components/stats/`)
   - Separate stats for each mining mode
   - Mining history storage (60-minute circular buffer)
   - Bitcoin: blocks checked, best difficulty, valid shares
   - Duino-Coin: shares accepted/rejected, DUCO earned (daily/total)
   - Hashrate calculation and averaging
   - Uptime tracking per mode

7. **Mining Coordinator** (`main/mining_coordinator.c`)
   - Manages mode switching between Bitcoin and Duino-Coin
   - Starts/stops appropriate mining engine
   - Routes stats updates to display
   - Handles configuration changes

### Hardware Pin Mapping

**Display (RGB)**: VSYNC=41, HSYNC=39, DE=40, PCLK=42, RGB_DATA=8,3,46,9,1,5,6,7,15,16,4,45,48,47,21,14, BL=2
**Touch (I2C)**: SDA=19, SCL=20, INT=18, RST=38

### Configuration Files

- `config.h.example` - Template with placeholder values (committed)
- `config.h` - Actual secrets (gitignored, created from template)
- Settings also configurable via web interface at `http://192.168.4.1` (AP mode) or device IP (station mode)

## Development Notes

### ESP-IDF Specifics

- Components are self-contained in `components/` directory
- Each component has its own `CMakeLists.txt` and include directory
- Main application in `main/` directory
- Use FreeRTOS tasks for concurrent operations (mining, display, networking)
- PSRAM required for LVGL frame buffers and mining datasets

### LVGL Integration

- Frame buffer in PSRAM due to size (800×480×2 = 768KB)
- Use hardware-accelerated DMA where possible
- Flush callback updates RGB interface
- Touch driver integrated with LVGL input device

### Mining Implementation

**Bitcoin Mode**:
- Uses ESP32 hardware SHA-256 accelerator for optimal performance
- Double SHA-256 for Bitcoin block hashing
- Stratum protocol for solo pool connection (e.g., public-pool.io)
- Expected hashrate: 40-50 KH/s
- Lottery mining: extremely low probability (~0.00000001%), but educational value
- Block finding expected time: ~31 billion years (but non-zero chance!)

**Duino-Coin Mode**:
- DUCO-S1 algorithm: SHA-1 based with server-provided challenges
- Kolka reward system: designed for low-power devices like ESP32
- Text-based protocol (simpler than JSON-RPC)
- Expected hashrate: 20-40 KH/s
- **Actually earns coins**: typically 1-5 DUCO per day (low value but real rewards)
- Full mining cycle experience within hours/days

**Why These Algorithms**:
- Both are memory-efficient (~256KB Bitcoin + ~100KB DUCO)
- Leverage ESP32 capabilities (hardware SHA-256, sufficient RAM)
- Proven implementations exist (NerdMiner V2, Duino-Coin ESP32)
- Monero RandomX rejected due to 256MB RAM requirement (ESP32 has only 8MB PSRAM)

### WiFi Behavior

1. On boot, attempt to connect to saved WiFi
2. If connection fails after 30 seconds, start AP mode
3. AP SSID: "ESP32-Miner-Setup", no password initially
4. Web interface at `http://192.168.4.1` for configuration
5. After saving WiFi credentials, reboot and connect

### Security Considerations

- Never commit `config.h`, `sdkconfig`, or files with wallet addresses
- Web interface should use basic authentication (implement in webserver component)
- Store sensitive data in NVS with encryption enabled

## Project Structure

```
osm-esp32/
├── main/                       # Main application entry point
│   └── mining_coordinator.c   # Mode switching coordinator
├── components/                 # Modular components
│   ├── mining_bitcoin/        # Bitcoin SHA-256 mining engine
│   ├── mining_duinocoin/      # Duino-Coin DUCO-S1 mining engine
│   ├── mining_common/         # Shared mining utilities
│   ├── display/               # Display + LVGL + touch
│   ├── wifi_manager/          # WiFi management
│   ├── webserver/             # HTTP server + config UI
│   ├── config/                # Configuration management
│   └── stats/                 # Statistics tracking
├── CMakeLists.txt             # Top-level build config
├── sdkconfig                  # ESP-IDF configuration (gitignored)
├── config.h.example           # Configuration template
└── partitions.csv             # Flash partition table
```

## Testing

**Bitcoin Mode Testing**:
1. Connect to public solo pool (e.g., public-pool.io, solo.ckpool.org)
2. Configure with Bitcoin wallet address
3. Monitor via serial console:
   - Stratum connection established
   - Mining jobs received
   - Blocks checked counter incrementing
   - Hashrate ~40-50 KH/s
   - Share submissions (if difficulty allows)

**Duino-Coin Mode Testing**:
1. Create account at https://duinocoin.com
2. Configure with username and optional mining key
3. Monitor via serial console:
   - Connection to server.duinocoin.com:2811
   - Job requests and responses
   - "GOOD" share acceptances
   - DUCO earnings accumulating
   - Hashrate ~20-40 KH/s

**Mode Switching Testing**:
- Switch between modes via touchscreen UI
- Switch via web interface
- Verify config persists after reboot
- Check stats separated per mode
- Ensure smooth transitions

## Reference Documentation

- [ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/latest/)
- [LVGL Documentation](https://docs.lvgl.io/)
- [NerdMiner V2](https://github.com/BitMaker-hub/NerdMiner_v2) - Bitcoin mining reference implementation
- [Duino-Coin](https://github.com/revoxhere/duino-coin) - DUCO-S1 reference implementation
- [Bitcoin Stratum Protocol](https://en.bitcoin.it/wiki/Stratum_mining_protocol)
- [Duino-Coin Getting Started](https://duinocoin.com/getting-started.html)
- Hardware repository: https://github.com/VIEWESMART/UEDX80480070ESP32-7inch-Touch-Display

## Secrets Management

The following files contain secrets and MUST NOT be committed:
- `config.h` - WiFi credentials, Bitcoin wallet, Duino-Coin username/key
- `sdkconfig` - May contain custom network settings
- `build/` - Build artifacts
- `spec.md` - Contains planning notes
- `IMPLEMENTATION_PLAN.md` - Contains implementation details

Always use `config.h.example` as the template and have users create their own `config.h`.

## Key Design Decisions

**Why NOT Monero?**
- RandomX requires 256 MB RAM minimum (even in light mode)
- ESP32-S3 has only 8 MB PSRAM total
- Memory requirement is 32x larger than available
- Physically impossible to implement on this hardware

**Why Bitcoin + Duino-Coin?**
- Bitcoin: Educational value, lottery excitement, proven NerdMiner implementation
- Duino-Coin: Actual rewards, complete mining cycle experience, designed for ESP32
- Both: Memory efficient, leverage hardware capabilities, complementary learning experiences
- Combined: Best of both worlds - lottery thrill + practical results
