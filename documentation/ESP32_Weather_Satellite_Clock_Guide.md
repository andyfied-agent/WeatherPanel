# ESP32 Weather Satellite Clock
## Integration Guide for ESP32-S3-Touch-LCD-3.49

This document provides complete integration instructions for the ESP32 Weather Satellite Clock project on the ESP32-S3-Touch-LCD-3.49 device.

---

## Table of Contents

1. [Hardware Overview](#hardware-overview)
2. [Wiring and Connections](#wiring-and-connections)
3. [Software Setup](#software-setup)
4. [Library Installation](#library-installation)
5. [Project Configuration](#project-configuration)
6. [Firmware Compilation and Flashing](#firmware-compilation-and-flashing)
7. [Configuration and Testing](#configuration-and-testing)
8. [Troubleshooting](#troubleshooting)

---

## Hardware Overview

### ESP32-S3-Touch-LCD-3.49 Specifications

| Component | Specification |
|-----------|--------------|
| **Display Controller** | AXS15231B |
| **Resolution** | 172 × 640 pixels (portrait) |
| **Interface** | QSPI (display), I2C (touch) |
| **Processor** | ESP32-S3R8 Dual-core 240MHz |
| **Flash** | 16MB QSPI NOR Flash |
| **PSRAM** | 8MB Octal PSRAM |
| **Touch** | Capacitive, I2C interface |

### Board Versions

- **V1**: Original release
- **V2**: Released after June 8, 2026
  - Some GPIO pins are swapped between V1 and V2
  - Check PCB for "Rev1.1" marking

**Important**: This guide uses V2 GPIO assignments. If you have V1, adjust pins accordingly.

---

## Wiring and Connections

### The ESP32-S3-Touch-LCD-3.49 is an **integrated development board**

All components are pre-connected on a single PCB. No external wiring is required for basic operation.

### GPIO Pin Assignments (V2)

| Function | GPIO Pin | Notes |
|----------|----------|-------|
| **QSPI CS** | 45 | Chip Select |
| **QSPI CLK** | 47 | Clock |
| **QSPI D0** | 39 | Data 0 |
| **QSPI D1** | 48 | Data 1 |
| **QSPI D2** | 40 | Data 2 |
| **QSPI D3** | 21 | Data 3 |
| **Touch SDA** | 4 | I2C Data |
| **Touch SCL** | 8 | I2C Clock |
| **Touch INT** | 1 | Interrupt (V2 only) |
| **Backlight** | 46 | PWM control |
| **Reset** | 4 | Shared LCD/Touch reset |
| **TE** | 2 | Touch Enable |

### SDMMC Card Interface

The SD card slot on the board uses dedicated SDMMC pins:

| Function | GPIO |
|----------|------|
| CMD | 23 |
| CLK | 22 |
| DAT0 | 21 |
| DAT1 | 19 |
| DAT2 | 20 |
| DAT3 | 18 |

### Expansion Headers

#### 22-pin Header (2.54mm pitch)
- GPIO expansion pins
- I2C interface
- UART for debugging

#### 15-pin Header (1mm pitch)
- 7× GPIO pins
- I2C, UART
- VBUS, 3.3V, GND

---

## Software Setup

### Option 1: Arduino IDE (Recommended)

#### Step 1: Install Arduino IDE

```bash
# Download from https://www.arduino.cc/en/software
# Or use package manager (Ubuntu/Debian):
wget https://www.arduino.cc/download.php?f=/arduino-ide_2.3.2_linux64.tar.xz
tar -xf arduino-ide_2.3.2_linux64.tar.xz
cd arduino-ide_2.3.2 && ./ArduinoIDE
```

#### Step 2: Install ESP32 Board Support

1. Open Arduino IDE
2. Go to **File** → **Preferences**
3. In "Additional Board Manager URLs", add:
   ```
   https://arduino.github.io/esp32/package_esp32_index.json
   ```
4. Go to **Tools** → **Board** → **Boards Manager**
5. Search for "esp32" and install "esp32 by Espressif Systems"

#### Step 3: Install Arduino_GFX Library

1. Go to **Sketch** → **Include Library** → **Manage Libraries**
2. Search for "Arduino GFX"
3. Install "Arduino GFX" by Xiao-Ming

#### Step 4: Install Other Required Libraries

- **JPEGDEC** (usually included with Arduino_GFX)
- **U8g2** (for fonts): Search and install "U8g2" by Olaf Schmidt

---

## Library Installation

The following libraries are required for the Weather Satellite Clock:

### Core Libraries

| Library | Purpose | Version |
|---------|---------|---------|
| Arduino_GFX | Display driver (QSPI, AXS15231B) | Latest |
| JPEGDEC | JPEG image decoding | Built-in |
| U8g2 | Font rendering | Latest |
| WiFiMulti | Multi-network WiFi | Built-in |

### Installation via Library Manager

1. **Arduino GFX**:
   - Search: "Arduino GFX"
   - Author: Xiao-Ming
   - Install

2. **U8g2**:
   - Search: "U8g2"
   - Author: Olaf Schmidt
   - Install

### Manual Installation (if needed)

```bash
cd ~/Arduino/libraries
git clone https://github.com/moononournation/Arduino_GFX.git
git clone https://github.com/OlafSchmidt/U8g2_for_Arduino.git
```

---

## Project Configuration

### 1. Configure WiFi Credentials

Open `Config.h` and update:

```cpp
#define SSID_NAME "YourWiFiNetwork"
#define SSID_PASSWORD "YourWiFiPassword"
```

### 2. Configure Timezone

The default is UTC+8 (Hong Kong). Adjust if needed:

```cpp
#define GMT_OFFSET_SEC (your_offset * 60 * 60)
```

### 3. Select Satellite Image Source

In `WeatherSatelliteImageClock.ino`, line 13-14:

```cpp
#include "https_nmc_cn.h"    // China National Meteorological Center
// #include "https_met_ie.h"  // Met Éireann, Ireland (alternative)
```

**Choose only ONE source by uncommenting one line.**

### 4. Board Configuration in Arduino IDE

Go to **Tools** → **Board** → **ESP32 Arduino**:

- **Board**: ESP32S3 Dev Module
- **Partition Scheme**: Huge APP (8MB SPIFFS)
- **PSRAM**: Octal PSRAM
- **CPU Frequency**: 240MHz
- **Flash Frequency**: 80MHz

---

## Firmware Compilation and Flashing

### Step 1: Open the Project

1. Open Arduino IDE
2. Go to **File** → **Open**
3. Navigate to `~/opt/WeatherPanel/WeatherSatelliteImageClock/`
4. Select `WeatherSatelliteImageClock.ino`

### Step 2: Verify Compilation

1. Go to **Sketch** → **Verify/Compile**
2. Check for errors in the console
3. Common issues to fix:
   - Wrong board selected
   - Missing libraries
   - Incorrect pin definitions

### Step 3: Connect the Device

1. Connect ESP32-S3-Touch-LCD-3.49 to computer via USB-C
2. Wait for device to appear as serial port
3. Check port permissions (if on Linux):
   ```bash
   sudo usermod -a -G dialout $USER
   ```

### Step 4: Flash the Firmware

1. Select correct port: **Tools** → **Port** → `/dev/ttyUSB0` (or `/dev/ttyACM0`)
2. Click **Upload** button (right arrow)
3. Hold the **BOOT** button on the device during upload if needed
4. Wait for "Upload Complete" message

### Step 5: Reset the Device

1. Press the **RESET** button on the board
2. Check serial monitor for output:
   ```
   Weather Satellite Image Clock
   SD_MMC Card Mounted...
   Waiting for WiFi to connect...
   ```

---

## Configuration and Testing

### Testing Display

After flashing, the display should:
1. Show "Weather Satellite Image Clock"
2. Display "SD_MMC Card Mounted" if SD card is inserted
3. Show WiFi connection progress
4. Display date and time

### Testing WiFi

Check the serial output for:
```
Waiting for WiFi to connect...connected
```

### Testing SD Card

The SD card must be formatted as FAT32 with the following structure:
```
/
└── fy4b/
    └── 2024/
        └── 01/
            └── 15/
                └── 1400.jpg
```

### Testing Satellite Image Download

1. Ensure WiFi is connected
2. Check serial output for download status
3. Images should be saved to SD card every 15 minutes
4. The clock displays a 8-hour timelapse

### Serial Monitor Configuration

- Baud Rate: 115200
- Line Ending: Both newline and carriage return
- UTF-8 encoding

---

## Troubleshooting

### Display Shows Noise/Static

**Symptoms**: Random patterns, no visible image

**Solutions**:
1. Verify board version (V1 vs V2) and adjust GPIO pins
2. Check PSRAM is enabled in Arduino IDE
3. Try different clock speeds (start at 10MHz)
4. Verify QSPI pin assignments match your board

### WiFi Not Connecting

**Symptoms**: "Waiting for WiFi" message never completes

**Solutions**:
1. Verify SSID and password are correct
2. Check WiFi signal strength
3. Ensure network allows HTTPS connections
4. Try different WiFi network

### SD Card Mount Fails

**Symptoms**: "SD_MMC Card Mount Failed"

**Solutions**:
1. Format SD card as FAT32 (not exFAT or NTFS)
2. Ensure card is properly inserted
3. Try different SD card
4. Check card capacity (recommended: 4-32GB)

### Images Not Downloading

**Symptoms**: No satellite images appear

**Solutions**:
1. Verify internet connection
2. Check firewall settings
3. Verify satellite URL is accessible
4. Check SD card has free space
5. Verify certificate is correctly configured

### Touch Not Responding

**Symptoms**: Touch input doesn't work

**Note**: The Weather Satellite Clock doesn't use touch functionality. Touch is only needed for interactive applications.

### Backlight Issues

**Symptoms**: Display visible but no backlight, or flickering

**Solutions**:
1. Verify backlight pin is correct (46 for V2, 32 for V1)
2. Check if `GFX_BL` is defined in Config.h
3. Verify PWM is working

### Compilation Errors

**Common Errors**:

1. **"Arduino_GFX_Library.h: No such file or directory"**
   - Install Arduino_GFX library

2. **"U8g2.h: No such file or directory"**
   - Install U8g2 library

3. **"esp32s3 does not support PSRAM octal"**
   - Select correct partition scheme: "Huge APP (8MB SPIFFS)"

4. **"gpio_config: invalid gpio number"**
   - Check GPIO pin assignments in Config.h

---

## Additional Features

### Weather API Integration (Optional)

To integrate real-time weather data:

1. Sign up for WeatherAPI: https://www.weatherapi.com/
2. Get API key
3. Add weather data fetch functionality to the sketch

### GPS Integration (Optional)

To integrate GPS for location-based weather:

1. Connect GPS module via UART
2. Use TinyGPS++ library
3. Fetch weather based on GPS coordinates

### Custom Satellite Source

To use a different satellite image source:

1. Edit `https_nmc_cn.h` or create new source file
2. Update URL template
3. Adjust timezone offset
4. Test with manual downloads

---

## Project Files

```
~/opt/WeatherPanel/WeatherSatelliteImageClock/
├── WeatherSatelliteImageClock.ino  # Main sketch
├── Config.h                          # Configuration file
├── HTTPS.h                           # HTTPS download functions
├── FILESYSTEM.h                      # Filesystem utilities
├── JPEG.h                            # JPEG decoding
├── https_nmc_cn.h                    # NMC satellite source
├── https_met_ie.h                    # Met Éireann source (alternative)
└── JPEG.h                            # JPEG decoder interface
```

---

## Performance Tips

1. **Power**: Disable WiFi when not updating images
2. **Memory**: Use SPIFFS/LittleFS for SD card
3. **Display**: Reduce refresh rate to save power
4. **Network**: Cache images locally

---

## References

- **Instructables Project**: https://www.instructables.com/Live-Weather-Satellite-Image-Clock/
- **WaveShare Documentation**: https://docs.waveshare.com/ESP32-S3-Touch-LCD-3.49
- **Arduino_GFX Library**: https://github.com/moononournation/Arduino_GFX
- **ESP32 Arduino Core**: https://github.com/espressif/arduino-esp32

---

## Support

For issues or questions:
1. Check the troubleshooting section above
2. Review the ESP32-S3-Touch-LCD-3.49_GUIDE.md
3. Search the Arduino forum
4. Check the Waveshare community forum

---

*Last Updated: 2024*
