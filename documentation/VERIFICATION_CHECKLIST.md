# ESP32 Weather Satellite Clock - Verification Checklist

This checklist helps verify that the ESP32 Weather Satellite Clock is properly configured and ready for deployment on the ESP32-S3-Touch-LCD-3.49 device.

---

## Pre-Installation Verification

### Hardware Check

- [ ] ESP32-S3-Touch-LCD-3.49 board received and undamaged
- [ ] USB-C cable is data-capable (not charge-only)
- [ ] SD card slot is functional (no debris)
- [ ] SD card available (4-32GB, FAT32 formatted)
- [ ] Board version identified (V1 or V2)
  - V2 has "Rev1.1" marking on PCB
- [ ] USB-C port shows power when connected to PC

### Software Environment Check

- [ ] Arduino IDE installed (version 1.8+ or 2.x)
- [ ] Arduino IDE opens without errors
- [ ] Python 3 is available (for helper scripts)
- [ ] Terminal access is functional
- [ ] Serial ports are accessible (`/dev/ttyUSB*` or `/dev/ttyACM*`)

---

## Arduino IDE Setup Verification

### ESP32 Board Support

- [ ] ESP32 Board Manager URL added to preferences
- [ ] esp32 package installed (check in Boards Manager)
- [ ] "ESP32S3 Dev Module" appears in Tools > Board menu
- [ ] "Huge APP (8MB SPIFFS)" partition scheme available
- [ ] "Octal PSRAM" option available in Tools menu

### Library Installation

- [ ] Arduino GFX library installed (by Xiao-Ming)
- [ ] U8g2 library installed (by Olaf Schmidt)
- [ ] Libraries appear in Sketch > Include Library menu
- [ ] No library dependency errors

### Environment Verification

```bash
# Check Python version
python3 --version  # Should show 3.10 or later

# Check if serial port exists (when device connected)
ls /dev/ttyUSB* /dev/ttyACM*

# Check user permissions
id | grep dialout  # Should show user in dialout group
```

---

## Project Configuration Verification

### File Structure Check

```bash
# Verify project files exist
ls -la ~/opt/WeatherPanel/WeatherSatelliteImageClock/

# Should see:
# ✓ WeatherSatelliteImageClock.ino
# ✓ Config.h
# ✓ HTTPS.h
# ✓ FILESYSTEM.h
# ✓ JPEG.h
# ✓ https_nmc_cn.h
# ✓ https_met_ie.h
# ✓ board_config.json
```

### Config.h Verification

```cpp
// Check Config.h content
cat ~/opt/WeatherPanel/WeatherSatelliteImageClock/Config.h

// Verify:
// ✓ SSID_NAME is set (not "YourWiFiNetwork")
// ✓ SSID_PASSWORD is set (not "YourWiFiPassword")
// ✓ GMT_OFFSET_SEC matches your timezone
// ✓ QSPI pins match your board version
// ✓ Display dimensions are correct (172x640)
```

### WiFi Credentials Verification

- [ ] SSID name is correct (case-sensitive)
- [ ] Password is correct
- [ ] WiFi network is accessible from ESP32 location
- [ ] Network allows HTTPS connections
- [ ] 2.4GHz WiFi is available (ESP32-S3 doesn't support 5GHz)

---

## Board Configuration Verification

### QSPI Pin Mapping (V2 Board)

| Signal | Expected GPIO | Verify |
|--------|--------------|--------|
| CS | 45 | ✓ Config.h shows 45 |
| CLK | 47 | ✓ Config.h shows 47 |
| D0 | 39 | ✓ Config.h shows 39 |
| D1 | 48 | ✓ Config.h shows 48 |
| D2 | 40 | ✓ Config.h shows 40 |
| D3 | 21 | ✓ Config.h shows 21 |

### Board Version Verification

- [ ] V2 board: GPIO 46 for backlight, GPIO 4 for reset
- [ ] V1 board: GPIO 32 for backlight, GPIO 3 for reset
- [ ] Config.h matches your board version

---

## Compilation Verification

### Build Check

1. Open Arduino IDE
2. File → Open → `~/opt/WeatherPanel/WeatherSatelliteImageClock/WeatherSatelliteImageClock.ino`
3. Select correct board: **ESP32S3 Dev Module**
4. Select partition scheme: **Huge APP (8MB SPIFFS)**
5. Select PSRAM: **Octal PSRAM**
6. **Sketch** → **Verify/Compile**

### Expected Output (Success)

```
Sketch uses 1234567 bytes (xx%) of program storage space.
Maximum is 16777216 bytes.
Variable x.y is used 1234 bytes.
Maximum is 16777216 bytes.
```

### Expected Output (Error)

- Error message with line number
- Missing library error
- Undefined symbol error
- Board selection error

**If errors occur, check:**
- [ ] All required libraries are installed
- [ ] Config.h is properly formatted
- [ ] Board selection is correct
- [ ] No syntax errors in source files

---

## Serial Port Verification

### Port Detection

```bash
# Connect ESP32 to PC via USB-C
ls /dev/ttyUSB* /dev/ttyACM*

# Should show something like:
# /dev/ttyUSB0
# /dev/ttyACM0
```

### Port Permissions

```bash
# Check user permissions
ls -la /dev/ttyUSB0

# If permission denied, add to dialout group:
sudo usermod -a -G dialout $USER

# Log out and log back in, then verify:
id | grep dialout
```

---

## Flashing Verification

### Upload Process

1. Select correct port: **Tools** → **Port** → `/dev/ttyUSB0` (or your port)
2. Click **Upload** button
3. Hold BOOT button if needed during upload
4. Wait for "Upload Complete" message

### Expected Behavior

- Serial monitor shows boot messages
- Display shows initial splash screen
- Progress bars appear on display
- "Weather Satellite Image Clock" text appears

### Upload Errors

**Error: "Failed to connect to ESP32"**
- Verify BOOT button procedure (hold BOOT, press RESET, release BOOT)
- Verify USB cable is functional
- Try different USB port

**Error: "esp_upload_load_files: Failed to load"**
- Verify partition scheme is correct
- Verify board selection is correct

---

## Post-Flash Verification

### Display Check

After flashing and resetting:

- [ ] Display shows "Weather Satellite Image Clock"
- [ ] Display shows "SD_MMC Card Mounted" (if SD card is present)
- [ ] Display shows "Waiting for WiFi to connect..."
- [ ] Display shows "connected" (if WiFi is successful)
- [ ] Date and time are displayed correctly

### Serial Monitor Check

```bash
# Open serial monitor (Tools > Serial Monitor)
# Baud rate: 115200

# Expected output:
Weather Satellite Image Clock
SD_MMC Card Mounted, space usage: XXX / XXX MB
Waiting for WiFi to connect...connected
Current time: Sun Jan 15 14:30:00 2024
```

### WiFi Check

- [ ] WiFi connection status shows "connected"
- [ ] Date/time are synchronized (NTP)
- [ ] Serial monitor shows successful NTP sync
- [ ] No repeated WiFi connection attempts

### SD Card Check

- [ ] SD card is mounted ("SD_MMC Card Mounted")
- [ ] Directory structure is created on SD card
- [ ] Satellite images are downloaded (check serial output)
- [ ] Images appear on display after download

---

## Satellite Image Verification

### Download Status

Check serial monitor for download messages:

```
[HTTPS] begin...
[HTTPS] GET... code: 200
https_fs_download(...)
```

### Image Display

- [ ] Satellite images appear on display
- [ ] Timelapse shows 8-hour window
- [ ] Images update every 15 minutes
- [ ] Date and time are overlaid on images

### Image Source

- [ ] Correct satellite source is configured (NMC or Met Éireann)
- [ ] Image timestamps match expected delay (45 minutes)
- [ ] Image quality is acceptable
- [ ] No download errors in serial output

---

## Performance Verification

### Memory Usage

Check serial output for memory status:

```
ESP.getFreeHeap(): XXXX, ESP.getFreePsram(): XXXX
```

**Expected values**:
- Free Heap: > 50,000 bytes
- Free PSRAM: > 1,000,000 bytes

### Display Refresh

- [ ] Display updates smoothly
- [ ] No flickering or artifacts
- [ ] Backlight is functional
- [ ] Colors are correct

### Update Interval

- [ ] Images download at expected interval (15 minutes)
- [ ] Clock display updates at expected frequency
- [ ] No excessive power consumption

---

## Troubleshooting Reference

### Issue: Display Shows Nothing

**Checklist**:
1. Verify board version (V1 vs V2)
2. Verify QSPI pin assignments
3. Verify PSRAM is enabled
4. Check display initialization code
5. Verify backlight GPIO is correct

### Issue: WiFi Not Connecting

**Checklist**:
1. Verify SSID and password
2. Check WiFi signal strength
3. Verify 2.4GHz network
4. Check firewall settings
5. Verify NTP server is accessible

### Issue: SD Card Mount Failed

**Checklist**:
1. Format card as FAT32
2. Verify card capacity (4-32GB)
3. Check card contacts
4. Verify SDMMC pin assignments
5. Try different SD card

### Issue: Images Not Downloading

**Checklist**:
1. Verify internet connection
2. Check satellite URL accessibility
3. Verify certificate is valid
4. Check firewall settings
5. Verify SD card has space

---

## Final Verification Summary

### Pre-Deployment Checklist

- [ ] All hardware components are connected and functional
- [ ] Software environment is properly configured
- [ ] WiFi credentials are correct
- [ ] Configuration files are updated
- [ ] Project compiles without errors
- [ ] Firmware is successfully flashed
- [ ] Display shows correct output
- [ ] WiFi connection is stable
- [ ] SD card is mounted
- [ ] Satellite images are updating

### Post-Deployment Checklist

- [ ] Device operates continuously without restarts
- [ ] Satellite images update as expected
- [ ] Date/time remain synchronized
- [ ] Power consumption is acceptable
- [ ] No error messages in serial output
- [ ] Display quality is satisfactory

---

## Support Resources

- **Integration Guide**: `~/src/workstation/ESP32_Weather_Satellite_Clock_Guide.md`
- **Hardware Wiring**: `~/src/workstation/HARDWARE_WIRING_REFERENCE.md`
- **Optimization Guide**: `~/src/workstation/OPTIMIZATION_GUIDE.md`
- **Customization Guide**: `~/src/workstation/CUSTOMIZATION_GUIDE.md`
- **Project README**: `~/src/workstation/WEATHER_SATELLITE_CLOCK_README.md`

---

*Last Updated: 2024*
