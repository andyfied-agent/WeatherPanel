# ESP32 Weather Satellite Clock - Optimized Configuration

This document provides optimization settings and troubleshooting tips for the ESP32 Weather Satellite Clock.

---

## Power Optimization

### Recommended Power Settings

For battery operation or power-sensitive deployments:

```cpp
// In Config.h, add:
#define POWER_SAVE_MODE true
#define BACKLIGHT_DIMMED true
#define WIFI_DPS_ENABLED true
```

### Deep Sleep Configuration

To reduce power consumption when not actively updating:

```cpp
// Enable deep sleep between updates
#define DEEP_SLEEP_ENABLED false  // Set to true for battery operation
#define SLEEP_DURATION_SEC 3600   // Sleep for 1 hour between updates
```

**Power Consumption Estimates**:
- **Active WiFi + Display**: ~300mA
- **Display Only**: ~150mA
- **Deep Sleep**: ~10mA
- **Deep Sleep with WiFi Off**: ~5mA

### Backlight Control

Adjust backlight brightness to save power:

```cpp
// In loop(), add brightness control
#define BACKLIGHT_PWM 128  // 50% brightness
```

---

## Performance Optimization

### Display Refresh Rate

Reduce unnecessary display updates:

```cpp
// Only update display when needed
#define UPDATE_DISPLAY_INTERVAL_SEC 60
```

### WiFi Connection Optimization

Use persistent WiFi connections:

```cpp
// Store WiFi credentials and reuse
#define WIFI_RECONNECT_ON_DISCONNECT true
#define WIFI_TIMEOUT_SEC 30
```

### SD Card Optimization

Use fast SD cards (UHS-I or faster):

- **Recommended**: Class 10 or U3
- **Capacity**: 8-32GB
- **Format**: FAT32

---

## Memory Optimization

### PSRAM Configuration

Ensure PSRAM is properly configured in Arduino IDE:

```
Tools > Partition Scheme > Huge APP (8MB SPIFFS)
Tools > PSRAM > Octal PSRAM
```

### Memory Usage Tips

- Monitor free memory with `ESP.getFreeHeap()` and `ESP.getFreePsram()`
- Use `malloc()` carefully for large buffers
- Consider using LittleFS instead of SPIFFS for better performance

---

## Network Optimization

### Certificate Handling

Reduce SSL handshake overhead:

```cpp
// Reuse connections when possible
#define KEEP_ALIVE_ENABLED true
#define CONNECTION_TIMEOUT_SEC 10
```

### Data Compression

Use gzip compression for downloads:

```cpp
// In HTTPS.h, add:
https.setRequestHeader("Accept-Encoding", "gzip");
```

### Update Interval Tuning

Adjust satellite image download frequency:

```cpp
// Default: 15 minutes
// Reduce to save bandwidth: 30 minutes
// Increase for more frequent updates: 10 minutes
#define UPDATE_INTERVAL_MINUTES 15
```

---

## Display Optimization

### Rotation Settings

For landscape orientation:

```cpp
#define DISPLAY_ROTATION 0  // Landscape
#define DISPLAY_WIDTH 640
#define DISPLAY_HEIGHT 172
```

### Color Depth Optimization

Use 8-bit color for reduced memory:

```cpp
// In Arduino_GFX initialization
#define COLOR_DEPTH_8 true
```

### Double Buffering

Enable double buffering for smoother updates:

```cpp
// In Config.h
#define DOUBLE_BUFFER true
```

---

## Troubleshooting Common Issues

### Issue: Out of Memory

**Symptoms**: Crashes, random restarts

**Solutions**:
1. Reduce display resolution
2. Lower image quality
3. Use smaller SD card cache
4. Enable PSRAM (8MB Octal)

### Issue: Slow WiFi Connection

**Symptoms**: "Waiting for WiFi" takes too long

**Solutions**:
1. Move closer to WiFi router
2. Use 2.4GHz WiFi (ESP32-S3 doesn't support 5GHz)
3. Reduce WiFi power management
4. Use static IP if possible

### Issue: Display Artifacts

**Symptoms**: Flickering, lines, noise

**Solutions**:
1. Verify QSPI clock speed (start with 10MHz)
2. Check power supply stability
3. Ensure proper grounding
4. Use shielded cables for external connections

### Issue: SD Card Read Errors

**Symptoms**: "SD_MMC Card Mount Failed"

**Solutions**:
1. Reformat as FAT32
2. Try different SD card
3. Check card contacts
4. Verify card capacity (4-32GB recommended)

### Issue: JPEG Decode Errors

**Symptoms**: "error = X" messages in serial monitor

**Solutions**:
1. Verify image is valid JPEG
2. Check image resolution matches display
3. Ensure proper color space (RGB565)
4. Verify JPEG source URL

---

## Calibration Procedures

### Display Brightness Calibration

1. Connect to serial monitor
2. Adjust brightness value in Config.h
3. Monitor for optimal visibility
4. Save preferred brightness level

### Touch Calibration (if using touch)

For touch applications:

```cpp
// Touch calibration coefficients
#define TOUCH_X_OFFSET 0
#define TOUCH_Y_OFFSET 0
#define TOUCH_X_MULTIPLIER 1.0
#define TOUCH_Y_MULTIPLIER 1.0
```

---

## Monitoring and Logging

### Enable Serial Debug Output

```cpp
// In Config.h
#define DEBUG_OUTPUT true
#define SERIAL_BAUD 115200
```

### Log File Configuration

Create log file on SD card:

```cpp
// In FILESYSTEM.h
#define LOG_FILE "/logs/clock.log"
```

### Remote Monitoring

Use MQTT for remote monitoring:

```cpp
#include <ESP32MQTT.h>

#define MQTT_SERVER "mqtt.example.com"
#define MQTT_PORT 1883
#define MQTT_CLIENT_ID "weather-clock-01"
```

---

## Firmware Updates

### OTA Update Support

Enable Over-The-Air updates:

```cpp
#include <ArduinoOTA.h>

void setupOTA() {
  ArduinoOTA.setHostname("weather-clock");
  ArduinoOTA.begin();
}

void loop() {
  ArduinoOTA.handle();
  // ... rest of loop
}
```

---

## Advanced Configuration

### Custom Satellite Sources

To add custom satellite sources:

1. Create new file in `https_*.h`
2. Define URL template
3. Configure timezone offset
4. Include in main sketch

### Weather API Integration

To add weather data:

```cpp
#include <HTTPClient.h>

const char* WEATHER_API_KEY = "your-api-key";
const char* WEATHER_API_URL = "http://api.weatherapi.com/v1/current.json";

void fetchWeather(String location) {
  HTTPClient http;
  http.begin(WEATHER_API_URL);
  http.addHeader("Key", WEATHER_API_KEY);
  int code = http.GET();
  // Parse and display weather data
}
```

### GPS Location Detection

To auto-detect location:

```cpp
#include <TinyGPS++.h>

TinyGPSPlus gps;

void loop() {
  // Read from GPS serial port
  // Update weather data based on location
}
```

---

## References

- [ESP32-S3-Touch-LCD-3.49 Guide](./ESP32-S3-Touch-LCD-3.49_GUIDE.md)
- [Hardware Wiring Reference](./HARDWARE_WIRING_REFERENCE.md)
- [ESP-IDF Documentation](https://docs.espressif.com/projects/esp-idf/)
- [Arduino GFX Library](https://github.com/moononournation/Arduino_GFX)

---

*Last Updated: 2024*
