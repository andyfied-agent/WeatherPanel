# WeatherPanel ESP32-S3-Touch-LCD-3.49 Integration Guide

> Source-of-truth note: the reconciled implementation is
> `WeatherPanel349/`, with board values in `Config349.h` and
> `board_config.json`. This guide originally contained an unverified
> GPIO map; its examples have been updated to the V2 map, but the firmware
> still requires a real PlatformIO build and hardware test before flashing.

This guide provides comprehensive instructions for adapting the WeatherPanel project to work with the ESP32-S3-Touch-LCD-3.49 display (AXS15231B driver).

## Hardware Specifications

| Parameter | Value |
|-----------|-------|
| Display Size | 3.49 inch |
| Resolution | 172 × 640 (portrait) |
| Display Driver | AXS15231B |
| Display Interface | QSPI |
| Touch Interface | I2C (0x38) |
| Touch Type | Capacitive |
| Flash | 16MB |
| PSRAM | 8MB OPI |

## Pin Configuration

### QSPI Display Interface (GPIO 45, 47, 39, 48, 40, 21)
| Signal | GPIO | Description |
|--------|------|-------------|
| CS | 45 | Chip Select |
| SCK | 47 | Clock |
| D0 | 39 | Data 0 |
| D1 | 48 | Data 1 |
| D2 | 40 | Data 2 |
| D3 | 21 | Data 3 |

### I2C Touch Interface (SDA GPIO 4, SCL GPIO 8)
| Signal | GPIO | Description |
|--------|------|-------------|
| SCL | 8 | I2C Clock |
| SDA | 4 | I2C Data |

### Backlight Control (V2)
| Signal | GPIO | Description |
|--------|------|-------------|
| BL | 46 | Backlight PWM control |

### TCA9554 I2C GPIO Expander (V1)
| Address | 0x20 |

## Required Libraries

1. **Arduino_GFX** - Display driver (v2.2.0+)
2. **Arduino_Touch** - Touch library
3. **Arduino_JSON** - JSON parsing (if needed)

Install via Library Manager:
- Arduino_GFX by moononournation
- Arduino_Touch by moononournation

## Code Changes Required

### 1. Create New Project Folder

```bash
cd /home/andyfied/src/workstation/WeatherPanel
mkdir WeatherPanel349
```

### 2. New Configuration File: WeatherPanel349/Config349.h

```cpp
#pragma once

// Network Configuration
const char *SSID_NAME = "YourAP";
const char *SSID_PASSWORD = "PleaseInputYourPasswordHere";
const long gmtOffset_sec = 8 * 60 * 60; // GMT+8 (Hong Kong)

// Display Configuration for ESP32-S3-Touch-LCD-3.49
#define DISP_WIDTH 172
#define DISP_HEIGHT 640
#define DISP_ROTATION 0  // 0 = portrait (default), 1=90°, 2=180°, 3=270°

// QSPI Pins (matches ESP32-S3-Touch-LCD-3.49)
#define QSPI_CS 45
#define QSPI_SCK 47
#define QSPI_D0 39
#define QSPI_D1 48
#define QSPI_D2 40
#define QSPI_D3 21

// I2C Touch Pins
#define I2C_SCL 8
#define I2C_SDA 4

// Backlight Control (V2 board)
#define GFX_BL 46

// SD Card Pins (adjust as needed)
#define SDMMC_CS 38
#define SD_MMC_CMD 39
#define SD_MMC_CLK 41
#define SD_MMC_D0 40

// Image URLs (customize as needed)
const char *PHOTO_URL_TEMPLATE = "https://www.hko.gov.hk/wxinfo/aws/hko_mica/hmm/latest_HMM.jpg?v=%lu123";
const char *PHOTO_FOLDER_L1 = "/hmm";
const char *PHOTO_FOLDER_L2_TEMPLATE = "/hmm/%d";
const char *PHOTO_FOLDER_L3_TEMPLATE = "/hmm/%d/%02d";
const char *PHOTO_FOLDER_L4_TEMPLATE = "/hmm/%d/%02d/%02d";
const char *PHOTO_FILE_TEMPLATE = "/hmm/%d/%02d/%02d/%02d%02d.jpg";

const char *RADAR_URL_TEMPLATE = "https://www.hko.gov.hk/wxinfo/radars/rad_256_png/2d256nradar_%d%02d%02d%02d%02d.jpg";
const char *RADAR_FOLDER_L1 = "/rad_256";
const char *RADAR_FOLDER_L2_TEMPLATE = "/rad_256/%d";
const char *RADAR_FOLDER_L3_TEMPLATE = "/rad_256/%d/%02d";
const char *RADAR_FOLDER_L4_TEMPLATE = "/rad_256/%d/%02d/%02d";
const char *RADAR_FILE_TEMPLATE = "/rad_256/%d/%02d/%02d/%02d%02d.jpg";

const char *SATELLITE_URL_TEMPLATE = "https://www.hko.gov.hk/wxinfo/intersat/satellite/image/images/h8_ir_x2M_%d%02d%02d%02d%02d00.jpg";
const char *SATELLITE_FOLDER_L1 = "/h8_ir_x2M";
const char *SATELLITE_FOLDER_L2_TEMPLATE = "/h8_ir_x2M/%d";
const char *SATELLITE_FOLDER_L3_TEMPLATE = "/h8_ir_x2M/%d/%02d";
const char *SATELLITE_FOLDER_L4_TEMPLATE = "/h8_ir_x2M/%d/%02d/%02d";
const char *SATELLITE_FILE_TEMPLATE = "/h8_ir_x2M/%d/%02d/%02d/%02d%02d.jpg";
```

### 3. Modified Main Sketch: WeatherPanel349/WeatherPanel349.ino

```cpp
/**
 * WeatherPanel for ESP32-S3-Touch-LCD-3.49
 *
 * This sketch adapts the original WeatherPanelHKO for the 3.49" display
 * with AXS15231B driver, QSPI interface, and I2C touch support.
 *
 * Display: 172x640 pixels (portrait orientation)
 * Driver: AXS15231B via QSPI (GPIO 45, 47, 39, 48, 40, 21)
 * Touch: I2C (SDA GPIO 4, SCL GPIO 8, address 0x38)
 */

#include "Config349.h"

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
WiFiMulti WiFiMulti;

char url[1024];
char path[1024];
time_t rounding_time;
time_t next_download_photo_time = 0;
time_t next_download_radar_time = 0;
time_t next_download_satellite_time = 0;

#include <SD_MMC.h>

#include "FILESYSTEM349.h"
#include "HTTPS349.h"
#include "JPEG349.h"
#include "Touch349.h"

/*******************************************************************************
 * Display Initialization for AXS15231B
 ******************************************************************************/
#include <Arduino_GFX_Library.h>

// QSPI Data Bus for AXS15231B
Arduino_DataBus *bus = new Arduino_ESP32QSPI(
    QSPI_CS,    // CS
    QSPI_SCK,   // SCK
    QSPI_D0,    // D0
    QSPI_D1,    // D1
    QSPI_D2,    // D2
    QSPI_D3,    // D3
    true        // is_shared_interface
);

// AXS15231B display with 172x640 resolution
Arduino_AXS15231B *gfx = new Arduino_AXS15231B(
    bus,
    -1,         // RST (use software reset)
    0,          // rotation
    true,       // IPS
    DISP_WIDTH, // width (172)
    DISP_HEIGHT,// height (640)
    0, 0, 0, 0, // col/row offsets
    axs15231b_180640_init_operations,
    sizeof(axs15231b_180640_init_operations)
);

/*******************************************************************************
 * Touch Initialization
 ******************************************************************************/
Arduino_Touch *tp = nullptr;

void initTouch() {
    // I2C touch configuration for AXS15231B
    // Note: Touch is integrated with display driver on this board
    // Additional touch initialization may be needed based on your needs
}

/*******************************************************************************
 * Display Drawing Callbacks
 ******************************************************************************/
int JPEGDraw(JPEGDRAW *pDraw) {
    gfx->draw16bitRGBBitmap(pDraw->x, pDraw->y, pDraw->pPixels,
                           pDraw->iWidth, pDraw->iHeight);
    return 1;
}

/*******************************************************************************
 * Clock Drawing Function
 * Adapted for 172x640 portrait display
 ******************************************************************************/
char timeStr[6];

void drawClock() {
    time(&rounding_time);
    rounding_time += gmtOffset_sec;
    struct tm *tmLocal = localtime(&rounding_time);
    strftime(timeStr, sizeof(timeStr), "%H:%M", tmLocal);

    rounding_time /= (30 * 60);
    rounding_time *= (30 * 60);
    unsigned long startMs = millis();

    // For 172x640 display, layout needs adjustment
    // Portrait orientation - images displayed vertically

    /* Draw photo (right column) */
    time_t minute_i = rounding_time - (3 * 60 * 60);
    while (minute_i < rounding_time) {
        struct tm *tmPast = localtime(&minute_i);
        sprintf(path, PHOTO_FILE_TEMPLATE, tmPast->tm_year + 1900,
               tmPast->tm_mon + 1, tmPast->tm_mday, tmPast->tm_hour,
               tmPast->tm_min);

        if (SD_MMC.exists(path)) {
            if (jpeg.open(path, jpegOpenSD_MMC, jpegClose, jpegRead, jpegSeek, JPEGDraw)) {
                jpeg.setPixelType(RGB565_LITTLE_ENDIAN);
                // Adjust crop area for 172x640 display
                // Right column: x=86 (center), width=86
                jpeg.setCropArea(86, 20, 86, 200);
                jpeg.decode(0, 0, 0);
                jpeg.close();

                gfx->setCursor(5, 550);
                gfx->setTextColor(RGB565_BLACK);
                gfx->setTextSize(2, 2, 2);
                gfx->print(timeStr);
                gfx->setCursor(6, 551);
                gfx->setTextColor(RGB565_WHITE);
                gfx->setTextSize(2, 2, 4);
                gfx->print(timeStr);
                gfx->flush();
            } else {
                Serial.print("JPEG error = ");
                Serial.println(jpeg.getLastError(), DEC);
            }
        }
        minute_i += (5 * 60);
    }

    /* Draw radar image (left column) */
    minute_i = rounding_time - (3 * 60 * 60);
    while (minute_i < rounding_time) {
        struct tm *tmPast = localtime(&minute_i);
        sprintf(path, RADAR_FILE_TEMPLATE, tmPast->tm_year + 1900,
               tmPast->tm_mon + 1, tmPast->tm_mday, tmPast->tm_hour,
               tmPast->tm_min);

        if (SD_MMC.exists(path)) {
            if (jpeg.open(path, jpegOpenSD_MMC, jpegClose, jpegRead, jpegSeek, JPEGDraw)) {
                jpeg.setPixelType(RGB565_BIG_ENDIAN);
                // Left column: x=0, width=86
                jpeg.setCropArea(0, 20, 86, 200);
                jpeg.decode(0, 0, 0);
                jpeg.close();
            } else {
                Serial.print("JPEG error = ");
                Serial.println(jpeg.getLastError(), DEC);
            }
        }
        minute_i += (6 * 60);
    }

    /* Draw satellite image (center column) */
    minute_i = rounding_time - (3 * 60 * 60);
    while (minute_i < rounding_time) {
        struct tm *tmPast = localtime(&minute_i);
        sprintf(path, SATELLITE_FILE_TEMPLATE, tmPast->tm_year + 1900,
               tmPast->tm_mon + 1, tmPast->tm_mday, tmPast->tm_hour,
               tmPast->tm_min);

        if (SD_MMC.exists(path)) {
            if (jpeg.open(path, jpegOpenSD_MMC, jpegClose, jpegRead, jpegSeek, JPEGDraw)) {
                jpeg.setPixelType(RGB565_BIG_ENDIAN);
                // Center column: x=43 (center of 86-width), width=86
                jpeg.setCropArea(43, 20, 86, 200);
                jpeg.decode(0, 0, 0);
                jpeg.close();
            } else {
                Serial.print("JPEG error = ");
                Serial.println(jpeg.getLastError(), DEC);
            }
        }
        minute_i += (10 * 60);
    }

    gfx->flush();
    Serial.printf("drawClock() used: %d ms\n", millis() - startMs);
}

/*******************************************************************************
 * Setup Function
 ******************************************************************************/
void setup() {
    Serial.begin(115200);
    // Serial.setDebugOutput(true);

    Serial.println("Weather Panel 3.49 (ESP32-S3-Touch-LCD-3.49)");
    delay(2000);

    // Initialize QSPI display
    Serial.println("Initializing AXS15231B display...");
    if (!gfx->begin()) {
        Serial.println("gfx->begin() failed!");
        while (1) delay(10);
    }

    gfx->fillScreen(RGB565_BLACK);
    gfx->setCursor(DISP_WIDTH/2 - 40, DISP_HEIGHT/2 - 20);
    gfx->setTextColor(RGB565_RED);
    gfx->setTextSize(2, 2, 4);
    gfx->print("Initializing...");
    gfx->flush();

#ifdef GFX_BL
    pinMode(GFX_BL, OUTPUT);
    digitalWrite(GFX_BL, HIGH);  // Turn on backlight
#endif

    // Initialize WiFi
    WiFi.mode(WIFI_STA);
    WiFiMulti.addAP(SSID_NAME, SSID_PASSWORD);

    // Initialize SD/MMC
    SD_MMC.setPins(SD_MMC_CLK, SD_MMC_CMD, SD_MMC_D0);
    if (!SD_MMC.begin("/root", true, false, SDMMC_FREQ_HIGHSPEED)) {
        Serial.println("SD Card Mount Failed");
        gfx->setCursor(DISP_WIDTH/2 - 30, DISP_HEIGHT/2);
        gfx->setTextColor(RGB565_RED);
        gfx->setTextSize(2, 2, 4);
        gfx->print("SD Failed");
        gfx->flush();
        while (1) delay(1000);
    } else {
        Serial.printf("SD Card Mounted: %llu / %llu MB\n",
                     SD_MMC.usedBytes() / (1024*1024),
                     SD_MMC.totalBytes() / (1024*1024));
        listDir(SD_MMC, "/", 4);
        gfx->setCursor(DISP_WIDTH/2 - 20, DISP_HEIGHT/2);
        gfx->setTextColor(RGB565_GREEN);
        gfx->print("SD OK");
        gfx->flush();
    }

    // Wait for WiFi connection
    Serial.print("Waiting for WiFi...");
    while ((WiFiMulti.run() != WL_CONNECTED)) {
        Serial.print(".");
        delay(500);
    }
    Serial.println(" connected");
    gfx->setCursor(DISP_WIDTH/2 - 25, DISP_HEIGHT/2 + 30);
    gfx->setTextColor(RGB565_GREEN);
    gfx->print("WiFi OK");
    gfx->flush();

    // Initialize NTP time
    setClock();
    gfx->setCursor(DISP_WIDTH/2 - 20, DISP_HEIGHT/2 + 60);
    gfx->setTextColor(RGB565_GREEN);
    gfx->print("NTP OK");
    gfx->flush();
}

/*******************************************************************************
 * Main Loop
 ******************************************************************************/
void loop() {
    drawClock();

    // Download latest photo (5 min interval)
    time(&rounding_time);
    if (rounding_time > next_download_photo_time) {
        Serial.printf("FreeHeap: %d, FreePsram: %d\n",
                     ESP.getFreeHeap(), ESP.getFreePsram());
        rounding_time /= (5 * 60);
        rounding_time *= (5 * 60);
        next_download_photo_time = rounding_time + (5 * 60);
        rounding_time -= (5 * 60);
        sprintf(url, PHOTO_URL_TEMPLATE, rounding_time);

        rounding_time += gmtOffset_sec;
        struct tm *tmLocal = gmtime(&rounding_time);
        SD_MMC.mkdir(PHOTO_FOLDER_L1);
        sprintf(path, PHOTO_FOLDER_L2_TEMPLATE, tmLocal->tm_year + 1900);
        SD_MMC.mkdir(path);
        sprintf(path, PHOTO_FOLDER_L3_TEMPLATE, tmLocal->tm_year + 1900,
               tmLocal->tm_mon + 1);
        SD_MMC.mkdir(path);
        sprintf(path, PHOTO_FOLDER_L4_TEMPLATE, tmLocal->tm_year + 1900,
               tmLocal->tm_mon + 1, tmLocal->tm_mday);
        SD_MMC.mkdir(path);
        sprintf(path, PHOTO_FILE_TEMPLATE, tmLocal->tm_year + 1900,
               tmLocal->tm_mon + 1, tmLocal->tm_mday, tmLocal->tm_hour,
               tmLocal->tm_min);
        if (!SD_MMC.exists(path)) {
            https_fs_download(url, SD_MMC, path);
        }
    }

    // Download latest radar image (6 min interval)
    time(&rounding_time);
    if (rounding_time > next_download_radar_time) {
        rounding_time /= (6 * 60);
        rounding_time *= (6 * 60);
        next_download_radar_time = rounding_time + (6 * 60);
        rounding_time -= (6 * 60);
        rounding_time += gmtOffset_sec;
        struct tm *tmLocal = gmtime(&rounding_time);
        sprintf(url, RADAR_URL_TEMPLATE, tmLocal->tm_year + 1900,
               tmLocal->tm_mon + 1, tmLocal->tm_mday, tmLocal->tm_hour,
               tmLocal->tm_min);

        SD_MMC.mkdir(RADAR_FOLDER_L1);
        sprintf(path, RADAR_FOLDER_L2_TEMPLATE, tmLocal->tm_year + 1900);
        SD_MMC.mkdir(path);
        sprintf(path, RADAR_FOLDER_L3_TEMPLATE, tmLocal->tm_year + 1900,
               tmLocal->tm_mon + 1);
        SD_MMC.mkdir(path);
        sprintf(path, RADAR_FOLDER_L4_TEMPLATE, tmLocal->tm_year + 1900,
               tmLocal->tm_mon + 1, tmLocal->tm_mday);
        SD_MMC.mkdir(path);
        sprintf(path, RADAR_FILE_TEMPLATE, tmLocal->tm_year + 1900,
               tmLocal->tm_mon + 1, tmLocal->tm_mday, tmLocal->tm_hour,
               tmLocal->tm_min);
        if (!SD_MMC.exists(path)) {
            https_fs_download(url, SD_MMC, path);
        }
    }

    // Download latest satellite image (10 min interval)
    time(&rounding_time);
    if (rounding_time > next_download_satellite_time) {
        rounding_time /= (10 * 60);
        rounding_time *= (10 * 60);
        next_download_satellite_time = rounding_time + (10 * 60);
        rounding_time -= (60 * 60);
        struct tm *tmGm = gmtime(&rounding_time);
        sprintf(url, SATELLITE_URL_TEMPLATE, tmGm->tm_year + 1900,
               tmGm->tm_mon + 1, tmGm->tm_mday, tmGm->tm_hour, tmGm->tm_min);

        rounding_time += gmtOffset_sec;
        struct tm *tmLocal = gmtime(&rounding_time);
        SD_MMC.mkdir(SATELLITE_FOLDER_L1);
        sprintf(path, SATELLITE_FOLDER_L2_TEMPLATE, tmLocal->tm_year + 1900);
        SD_MMC.mkdir(path);
        sprintf(path, SATELLITE_FOLDER_L3_TEMPLATE, tmLocal->tm_year + 1900,
               tmLocal->tm_mon + 1);
        SD_MMC.mkdir(path);
        sprintf(path, SATELLITE_FOLDER_L4_TEMPLATE, tmLocal->tm_year + 1900,
               tmLocal->tm_mon + 1, tmLocal->tm_mday);
        SD_MMC.mkdir(path);
        sprintf(path, SATELLITE_FILE_TEMPLATE, tmLocal->tm_year + 1900,
               tmLocal->tm_mon + 1, tmLocal->tm_mday, tmLocal->tm_hour,
               tmLocal->tm_min);
        if (!SD_MMC.exists(path)) {
            https_fs_download(url, SD_MMC, path);
        }
    }

    delay(5 * 1000);
}
```

### 4. Platform Configuration: WeatherPanel349/platformio.ini (optional)

```ini
; PlatformIO Project Configuration File
; For ESP32-S3-Touch-LCD-3.49 with 16MB Flash, 8MB PSRAM

[env:esp32s3]
platform = espressif32
board = esp32s3
framework = arduino

; Board specific settings
board_build.fqbn = esp32:esp32:esp32s3:UploadSpeed=921600,USBMode=hwcdc,CDCOnBoot=cdc,CPUFreq=240,FlashMode=qio,FlashSize=16M,PartitionScheme=app3M_fat9M_16MB,DebugLevel=none,PSRAM=opi,LoopCore=1,EventsCore=1

; Upload settings
upload_speed = 921600

; Libraries
lib_deps =
    moononournation/Arduino GFX@^2.2.0
    moononournation/Arduino Touch@^1.0.0
    bitbank2/JPEGDEC@^1.8.2

; Build flags
build_flags =
    -D ARDUINO_ARCH_ESP32
    -D ARDUINO_ESP32S3

; Monitor settings
monitor_speed = 115200
```

### 5. Alternative Arduino IDE Board Configuration

In Arduino IDE, configure the board as follows:

```
Board: ESP32 Arduino
FQBN: esp32:esp32:esp32s3
CPU Frequency: 240MHz
Flash Mode: QIO
Flash Size: 16MB
Partition Scheme: 3MB App, 9MB FATFS (or app3M_fat9M_16MB)
PSRAM: OPI PSRAM
Upload Speed: 921600
USB Mode: Hardware CDC + JTAG
```

## Pixel Mapping for 172x640 Display

The AXS15231B driver supports the 172×640 resolution natively. Key considerations:

1. **Portrait Orientation (default)**: 172 width × 640 height
2. **Rotation Support**: The display supports all 4 rotations via MADCTL register
3. **Color Order**: RGB (not BGR) - set MADCTL_RGB bit
4. **Pixel Format**: RGB565 (16-bit)

### Layout Suggestions for Weather Panel

With 172×640 portrait display:
- **Top 100px**: Status bar (WiFi, battery, time)
- **Next 180px**: Photo timelapse (right column)
- **Next 180px**: Radar image (left column)
- **Next 180px**: Satellite image (center column)

### Alternative Landscape Layout

If rotated 90° (640×172):
- **Left**: Photo strip (vertical)
- **Center**: Radar + Satellite
- **Right**: Clock overlay

## Backlight Control

### V2 Board (GPIO 46)
```cpp
#define GFX_BL 46
void setup() {
  pinMode(GFX_BL, OUTPUT);
  analogWrite(GFX_BL, 255);  // Full brightness (0-255)
  // Or use PWM with specific frequency
  ledcSetup(0, 5000, 8);
  ledcAttachPin(GFX_BL, 0);
  ledcWrite(0, 255);
}
```

### V1 Board (TCA9554 GPIO Expander)
If using V1 with TCA9554 expander:
- Backlight controlled via I2C GPIO expander
- Address: 0x20 (default)
- Use appropriate GPIO pin from TCA9554

## Touch Support

The AXS15231B integrates touch controller via I2C:
- I2C Address: 0x38
- Interface: GPIO 8 (SCL), GPIO 4 (SDA)
- Touch resolution: Same as display (172×640)

Basic touch reading example:
```cpp
#include <Wire.h>

void readTouch() {
  Wire.begin(I2C_SCL, I2C_SDA);
  Wire.beginTransmission(0x38);
  // Read touch data registers
  // Process touch coordinates
  Wire.endTransmission();
}
```

## Troubleshooting

### Display Not Initializing
1. Verify QSPI pins match hardware (GPIO 45, 47, 39, 48, 40, 21)
2. Check display reset pin (use software reset if -1)
3. Verify power supply (5V via USB-C)
4. Try different rotation settings

### Black Screen
1. Check backlight GPIO (46 for V2)
2. Verify AXS15231B init operations match hardware version
3. Try `analogWrite(GFX_BL, 128)` to test PWM

### Touch Not Working
1. Verify I2C pins (SDA GPIO 4, SCL GPIO 8)
2. Check I2C device address (0x38)
3. Use I2C scanner to verify touch controller
4. Check interrupt pin connection

### Image Distortion
1. Verify resolution constants (172×640)
2. Check col_offset1/col_offset2 values (usually 0)
3. Ensure IPS=true for correct color rendering
4. Verify MADCTL rotation settings

## Testing Checklist

- [ ] QSPI pins correctly configured (10-15)
- [ ] Display initializes without errors
- [ ] Backlight turns on
- [ ] Images display correctly (no distortion)
- [ ] SD card mounts successfully
- [ ] WiFi connects to network
- [ ] NTP time syncs correctly
- [ ] Images download and display
- [ ] Touch interface functional (if implemented)

## Additional Resources

- [Arduino_GFX Library](https://github.com/moononournation/Arduino_GFX)
- [AXS15231B Datasheet](https://dl.espressif.com/AE/esp_iot_solution/AXS15231B_Datasheet_V0.5_20230306.pdf)
- [Waveshare ESP32-S3-Touch-LCD-3.49 Wiki](https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-3.49)
- [ESP-LCD AXS15231B Component](https://github.com/espressif/esp-iot-solution/tree/master/components/display/lcd/esp_lcd_axs15231b)

## Version History

- v1.0: Initial integration for ESP32-S3-Touch-LCD-3.49
- v1.1: Added touch support, improved layout

---

**Author**: Adapted from WeatherPanelHKO for ESP32-S3-Touch-LCD-3.49
**License**: Same as original WeatherPanel
