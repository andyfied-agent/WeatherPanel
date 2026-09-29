# ESP32 Weather Satellite Clock Project

This repository contains the configuration and integration files for deploying the ESP32 Weather Satellite Clock on the **ESP32-S3-Touch-LCD-3.49** device.

## Overview

The ESP32 Weather Satellite Clock is a desktop display that shows:
- **Live weather satellite imagery** (from China NMC or alternative sources)
- **Current date and time**
- **8-hour satellite timelapse** (15-minute intervals)
- **WiFi connection status**
- **SD card status**

## Hardware Requirements

- **ESP32-S3-Touch-LCD-3.49** (WaveShare)
  - AXS15231B display controller
  - 172×640 pixel resolution (portrait)
  - 8MB Octal PSRAM
  - 16MB QSPI Flash
  - Capacitive touch (I2C)
  - SD card slot

## Software Requirements

- **Arduino IDE** (version 1.8+ or 2.x)
- **ESP32 Board Support** (from Arduino Boards Manager)
- **Arduino GFX Library** (for display driver)
- **U8g2 Library** (for font rendering)

## Quick Start

### 1. Install Arduino IDE

Download from https://www.arduino.cc/en/software

### 2. Install ESP32 Board Support

1. Open Arduino IDE
2. Go to **File** → **Preferences**
3. Add to "Additional Board Manager URLs":
   ```
   https://arduino.github.io/esp32/package_esp32_index.json
   ```
4. Go to **Tools** → **Board** → **Boards Manager**
5. Search for "esp32" and install "esp32 by Espressif Systems"

### 3. Install Libraries

1. Go to **Sketch** → **Include Library** → **Manage Libraries**
2. Search for and install:
   - **Arduino GFX** by Xiao-Ming
   - **U8g2** by Olaf Schmidt

### 4. Configure WiFi Credentials

Edit `~/opt/WeatherPanel/WeatherSatelliteImageClock/Config.h`:

```cpp
#define SSID_NAME "YourWiFiNetwork"
#define SSID_PASSWORD "YourWiFiPassword"
```

### 5. Flash Firmware

1. Open Arduino IDE
2. File → Open → `~/opt/WeatherPanel/WeatherSatelliteImageClock/WeatherSatelliteImageClock.ino`
3. Select board settings:
   - Board: **ESP32S3 Dev Module**
   - Partition Scheme: **Huge APP (8MB SPIFFS)**
   - PSRAM: **Octal PSRAM**
4. Connect ESP32-S3-Touch-LCD-3.49 via USB-C
5. Select port: **Tools** → **Port** → `/dev/ttyUSB0` (or `/dev/ttyACM0`)
6. Click **Upload** button
7. Hold BOOT button during upload if needed

## Project Structure

```
~/opt/WeatherPanel/WeatherSatelliteImageClock/
├── WeatherSatelliteImageClock.ino  # Main sketch (updated for ESP32-S3-Touch-LCD-3.49)
├── Config.h                          # Configuration file (NEW)
├── HTTPS.h                           # HTTPS download functions
├── FILESYSTEM.h                      # Filesystem utilities
├── JPEG.h                            # JPEG decoding
├── https_nmc_cn.h                    # NMC satellite source (default)
├── https_met_ie.h                    # Met Éireann source (alternative)
└── board_config.json                 # Board configuration (NEW)

~/src/workstation/
├── ESP32_Weather_Satellite_Clock_Guide.md      # Integration guide
├── HARDWARE_WIRING_REFERENCE.md                # Hardware wiring details
├── OPTIMIZATION_GUIDE.md                       # Performance tips
├── CUSTOMIZATION_GUIDE.md                      # Customization options
├── weather_panel_config.py                     # Configuration manager script
└── build_weather_clock.sh                      # Build and flash script
```

## Configuration

### Timezone

The default timezone is UTC+8 (Hong Kong). Adjust in Config.h:

```cpp
#define GMT_OFFSET_SEC (your_offset * 60 * 60)
```

### Satellite Source

Choose one source in the main sketch (line 13-14):

```cpp
#include "https_nmc_cn.h"    // China NMC (default)
// #include "https_met_ie.h" // Met Éireann (alternative)
```

### Display Orientation

Portrait (default):
```cpp
#define DISPLAY_WIDTH 172
#define DISPLAY_HEIGHT 640
#define DISPLAY_ROTATION 180
```

Landscape:
```cpp
#define DISPLAY_WIDTH 640
#define DISPLAY_HEIGHT 172
#define DISPLAY_ROTATION 0
```

## Troubleshooting

### Display Shows Nothing

- Verify board version (V1 vs V2) - check GPIO assignments
- Ensure PSRAM is enabled (Octal PSRAM in Arduino IDE)
- Check QSPI pin assignments match your board

### WiFi Not Connecting

- Verify SSID and password are correct
- Check WiFi signal strength
- Ensure network allows HTTPS connections
- Use 2.4GHz WiFi only (ESP32-S3 doesn't support 5GHz)

### SD Card Mount Failed

- Format SD card as FAT32 (not exFAT or NTFS)
- Ensure card capacity is 4-32GB
- Reseat the card

### Images Not Downloading

- Verify internet connection
- Check firewall settings
- Verify satellite URL is accessible
- Check SD card has free space

### Build/Compilation Errors

1. Install Arduino GFX library
2. Install U8g2 library
3. Select correct board (ESP32S3 Dev Module)
4. Select correct partition scheme (Huge APP)

## Build and Flash Script

Use the provided build script:

```bash
cd ~/src/workstation

# Generate Config.h
./build_weather_clock.sh --config

# Build the project
./build_weather_clock.sh --build

# Flash to default port
./build_weather_clock.sh --flash

# Flash to specific port
./build_weather_clock.sh -f -p /dev/ttyUSB1

# Monitor serial output
./build_weather_clock.sh --monitor
```

## SD Card Setup

1. Format SD card as FAT32
2. Insert into ESP32-S3-Touch-LCD-3.49
3. Power on the device
4. The firmware will automatically create the directory structure

### Directory Structure

```
/
└── fy4b/
    └── 2024/
        └── 01/
            └── 15/
                └── 1400.jpg
```

## Resources

- **Instructables Project**: https://www.instructables.com/Live-Weather-Satellite-Image-Clock/
- **WaveShare Documentation**: https://docs.waveshare.com/ESP32-S3-Touch-LCD-3.49
- **Arduino GFX Library**: https://github.com/moononournation/Arduino_GFX
- **ESP32 Arduino Core**: https://github.com/espressif/arduino-esp32
- **U8g2 Library**: https://github.com/olikraus/u8g2

## License

This project is based on the ESP32 Weather Satellite Clock by moononournation. See the original project for licensing details.

## Support

For issues or questions:
1. Check the troubleshooting section above
2. Review the integration guides in `~/src/workstation/`
3. Search the Arduino forum
4. Check the WaveShare community forum

---

*Last Updated: 2024*
