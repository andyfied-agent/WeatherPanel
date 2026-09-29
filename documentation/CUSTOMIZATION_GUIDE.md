# ESP32 Weather Satellite Clock - Configuration Guide

This guide explains how to configure the ESP32 Weather Satellite Clock for your specific needs.

---

## Quick Start

1. **Install Arduino IDE** (recommended)
2. **Install ESP32 Board Support**
3. **Install Required Libraries**
4. **Configure WiFi Credentials**
5. **Flash Firmware**

---

## Configuration Files

### Config.h - Main Configuration

Location: `WeatherSatelliteImageClock/Config.h`

This file contains all configuration settings for the clock.

#### WiFi Settings

```cpp
#define SSID_NAME "YourWiFiNetwork"
#define SSID_PASSWORD "YourWiFiPassword"
```

**Replace with your WiFi credentials**:
- Enter the exact WiFi network name (case-sensitive)
- Enter the WiFi password

#### Timezone Configuration

```cpp
#define GMT_OFFSET_SEC (8 * 60 * 60)
```

**Common Timezone Offsets**:

| Location | Offset (hours) | Code |
|----------|---------------|------|
| Hong Kong / China | +8 | `(8 * 60 * 60)` |
| London (GMT) | 0 | `(0 * 60 * 60)` |
| Paris (CET) | +1 | `(1 * 60 * 60)` |
| New York (EST) | -5 | `(-5 * 60 * 60)` |
| Los Angeles (PST) | -8 | `(-8 * 60 * 60)` |
| Tokyo | +9 | `(9 * 60 * 60)` |
| Sydney | +10 | `(10 * 60 * 60)` |

#### Display Settings

```cpp
#define DISPLAY_WIDTH 172
#define DISPLAY_HEIGHT 640
#define DISPLAY_ROTATION 180
#define DISPLAY_IPS false
```

**Portrait vs Landscape**:

- **Portrait** (default): Width=172, Height=640, Rotation=180
- **Landscape**: Width=640, Height=172, Rotation=0

#### Backlight Control

```cpp
#define GFX_BL 46
```

**GPIO pin for backlight**:
- V2 Board: GPIO 46
- V1 Board: GPIO 32

---

### https_nmc_cn.h - Satellite Source Configuration

Location: `WeatherSatelliteImageClock/https_nmc_cn.h`

This file defines the satellite image source.

#### URL Template

```cpp
const char *SATELLITE_URL_TEMPLATE = "https://image.nmc.cn/product/%d/%02d/%02d/WXBL/medium/SEVP_NSMC_WXBL_FY4B_ETCC_ACHN_LNO_PY_%d%02d%02d%02d%02d00000.JPG";
```

**Format placeholders**:
- Year, Month, Day for image location
- Year, Month, Day, Hour, Minute for filename

#### Time Settings

```cpp
// Satellite image is delayed 45 minutes from real time
// Update every 15 minutes
```

**To change delay**:
1. Edit `https_nmc_cn.h`
2. Adjust `SATELLITE_URL_TEMPLATE` with your source
3. Update timezone offset if needed

---

## Arduino IDE Configuration

### Board Settings

1. **Tools > Board > ESP32 Arduino**
2. Select: `ESP32S3 Dev Module`

### Partition Scheme

1. **Tools > Partition Scheme**
2. Select: `Huge APP (8MB SPIFFS)`

**Why Huge APP?**
- Provides 8MB for the app (firmware)
- Leaves 8MB for SPIFFS (SD card emulation)
- Recommended for this project

### PSRAM Configuration

1. **Tools > PSRAM**
2. Select: `Octal PSRAM`

**Why Octal PSRAM?**
- ESP32-S3R8 has 8MB octal PSRAM
- Required for display buffering
- Enables larger image rendering

### CPU Frequency

1. **Tools > CPU Frequency**
2. Select: `240MHz`

### Flash Frequency

1. **Tools > Flash Frequency**
2. Select: `80MHz`

---

## WiFi Configuration

### Static WiFi Settings (Optional)

For more stable connections, you can use static IP:

```cpp
#include <WiFi.h>
#include <WiFiMulti.h>

WiFiMulti WiFiMulti;

void setup() {
  // ... existing code ...

  // Set static IP
  IPAddress staticIP(192, 168, 1, 100);
  IPAddress gateway(192, 168, 1, 1);
  IPAddress subnet(255, 255, 255, 0);
  IPAddress dns(8, 8, 8, 8);

  WiFi.config(staticIP, dns, subnet, gateway);

  WiFiMulti.addAP(SSID_NAME, SSID_PASSWORD);
}
```

### Multiple WiFi Networks

The code already supports multiple WiFi networks:

```cpp
// WiFiMulti automatically connects to any available network
// Configure in Config.h
WiFiMulti.addAP(SSID_NAME, SSID_PASSWORD);
```

### Network Troubleshooting

**Issue**: WiFi not connecting

**Solutions**:
1. Verify WiFi credentials are correct
2. Check WiFi signal strength
3. Try different WiFi network (2.4GHz only)
4. Check router settings (no MAC filtering)

---

## Satellite Image Source Configuration

### Available Sources

1. **NMC (China)** - Default
   - Source: https://image.nmc.cn/
   - Images: FY4B satellite
   - Delay: 45 minutes
   - Update: Every 15 minutes

2. **Met Éireann (Ireland)** - Alternative
   - Uncomment `#include "https_met_ie.h"`
   - Comment out `#include "https_nmc_cn.h"`
   - May require different timezone settings

### Custom Satellite Source

To add a new satellite source:

1. Create new file: `https_custom.h`
2. Define URL template:
   ```cpp
   const char *SATELLITE_URL_TEMPLATE = "https://your-source.com/path/%d/%02d/%02d/image.jpg";
   ```
3. Update timezone offset
4. Include in main sketch

### Image Storage

Images are stored on SD card:
```
/fy4b/
  2024/
    01/
      15/
        1400.jpg
```

---

## SD Card Configuration

### SD Card Requirements

- **Capacity**: 4GB - 32GB (recommended)
- **Format**: FAT32
- **Speed**: Class 10 or U3

### SD Card Setup

1. Format as FAT32 using Disk Utility (macOS) or FAT32 Format (Windows)
2. Insert into ESP32-S3-Touch-LCD-3.49
3. Power on the device
4. Check serial output for "SD_MMC Card Mounted"

### SD Card Structure

The firmware creates this structure automatically:
```
/
└── fy4b/
    └── <year>/
        └── <month>/
            └── <day>/
                └── <hour><minute>.jpg
```

---

## Debug Configuration

### Enable Serial Debug Output

```cpp
#define DEBUG_OUTPUT true
#define SERIAL_BAUD 115200
```

### Serial Monitor Settings

- **Baud Rate**: 115200
- **Line Ending**: Both newline and carriage return
- **Encoding**: UTF-8

### Debug Output Examples

```
Weather Satellite Image Clock
SD_MMC Card Mounted, space usage: 500 / 7500 MB
Waiting for WiFi to connect...connected
Current time: Sun Jan 15 14:30:00 2024
```

---

## Power Configuration

### Backlight Control

Adjust backlight brightness:

```cpp
// In loop(), add after display initialization
#define BACKLIGHT_BRIGHTNESS 255  // 0-255 (255 = full brightness)
```

### Power Saving Mode

To save power (advanced):

```cpp
// Disable display when not needed
#define POWER_SAVE_MODE false
```

---

## Customization Examples

### Change Date/Time Font

```cpp
// In Config.h
#define DATE_FONT u8g2_font_selectric14_tr
#define TIME_FONT u8g2_font_logisoso92_tn
```

### Change Clock Position

```cpp
// In Config.h
#define DATE_CURSOR_X 50
#define DATE_CURSOR_Y 50
#define TIME_CURSOR_X 100
#define TIME_CURSOR_Y 200
```

### Change Colors

```cpp
// In Config.h
#define RGB565_CLOCK_COLOR 0x07E0  // Green
#define RGB565_DATE_COLOR 0x001F   // Blue
```

### Add Weather Data

To integrate weather API (requires Weather API key):

```cpp
// Add to WeatherSatelliteImageClock.ino
const char* WEATHER_API_KEY = "your-api-key";

void fetchWeather() {
  HTTPClient http;
  http.begin("http://api.weatherapi.com/v1/current.json?key=" + String(WEATHER_API_KEY));
  int code = http.GET();
  // Parse and display weather data
}
```

---

## FAQ

### Q: What satellite images are shown?
A: The clock shows FY4B satellite imagery from China National Meteorological Center. Images are 45 minutes old and updated every 15 minutes.

### Q: Can I use a different satellite source?
A: Yes, create a custom source file or use the alternative Met Éireann source.

### Q: How do I change the timezone?
A: Edit `GMT_OFFSET_SEC` in Config.h with your timezone offset.

### Q: What if WiFi fails?
A: The clock will retry automatically. Check serial output for error messages.

### Q: Can I add touch functionality?
A: The current firmware doesn't use touch, but the board supports it. Custom firmware can add touch support.

### Q: How do I update the firmware?
A: Connect via USB and re-upload using Arduino IDE, or enable OTA updates.

---

## Advanced Configuration

### Custom Display Initialization

For custom display initialization:

```cpp
// In Config.h, define custom init array
const uint8_t my_custom_init[] = {
  // Your custom initialization commands
};

Arduino_GFX *g = new Arduino_AXS15231B(
  bus,
  GFX_NOT_DEFINED,
  DISPLAY_ROTATION,
  DISPLAY_IPS,
  DISPLAY_WIDTH,
  DISPLAY_HEIGHT,
  0, 0, 0, 0,
  my_custom_init, sizeof(my_custom_init));
```

### Custom JPEG Source

For custom JPEG handling:

```cpp
// In JPEG.h, modify jpegOpenSD_MMC
void *jpegOpenSD_MMC(const char *filename, int32_t *size) {
  // Custom file opening logic
  // ...
}
```

---

## References

- [Arduino IDE Documentation](https://docs.arduino.cc/software/ide-v1/)
- [ESP32 Arduino Core](https://github.com/espressif/arduino-esp32)
- [Arduino GFX Library](https://github.com/moononournation/Arduino_GFX)
- [U8g2 Fonts](https://github.com/olikraus/u8g2/wiki/f)

---

*Last Updated: 2024*
