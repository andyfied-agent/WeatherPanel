#!/bin/bash
# WeatherPanel 3.49 Setup Script
# This script sets up the WeatherPanel for ESP32-S3-Touch-LCD-3.49

set -e

echo "=== WeatherPanel 3.49 Setup Script ==="
echo ""

# Define paths
PROJECT_ROOT="/home/andyfied/src/workstation/WeatherPanel"
PROJECT_DIR="$PROJECT_ROOT/WeatherPanel349"

# Check if files exist
echo "Checking project files..."
if [ ! -d "$PROJECT_DIR" ]; then
    echo "ERROR: Project directory not found: $PROJECT_DIR"
    exit 1
fi

FILES=(
    "Config349.h"
    "HTTPS349.h"
    "FILESYSTEM349.h"
    "JPEG349.h"
    "Touch349.h"
    "WeatherPanel349.ino"
)

for file in "${FILES[@]}"; do
    if [ -f "$PROJECT_DIR/$file" ]; then
        echo "  ✓ $file exists"
    else
        echo "  ✗ $file MISSING"
        exit 1
    fi
done

echo ""
echo "=== Verification Complete ==="
echo ""
echo "Files created:"
ls -lh "$PROJECT_DIR"
echo ""
echo "=== Next Steps ==="
echo ""
echo "1. Install Arduino IDE (https://www.arduino.cc/en/software)"
echo ""
echo "2. Add ESP32 board support in Arduino IDE:"
echo "   - Go to File > Preferences"
echo "   - In 'Additional Board Manager URLs', add:"
echo "     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-records/package_esp32_index.json"
echo "   - Go to Tools > Board > Boards Manager"
echo "   - Search for 'esp32' and install 'esp32 by Espressif Systems'"
echo ""
echo "3. Install required libraries via Library Manager:"
echo "   - Arduino GFX (by moononournation)"
echo "   - Arduino Touch (by moononournation)"
echo ""
echo "4. Configure Board Settings:"
echo "   - Board: ESP32 Arduino"
echo "   - Board Version: ESP32S3 Dev Module"
echo "   - FQBN: esp32:esp32:esp32s3"
echo "   - CPU Frequency: 240MHz"
echo "   - Flash Mode: QIO"
echo "   - Flash Size: 16MB"
echo "   - Partition Scheme: 3MB App, 9MB FATFS"
echo "   - PSRAM: OPI PSRAM"
echo "   - Upload Speed: 921600"
echo "   - USB Mode: Hardware CDC + JTAG"
echo ""
echo "5. Edit Config349.h to add your WiFi credentials:"
echo "   - Change SSID_NAME to your WiFi network name"
echo "   - Change SSID_PASSWORD to your WiFi password"
echo ""
echo "6. Select port and click Upload in Arduino IDE"
echo ""
echo "For PlatformIO users:"
echo "  - Create new project with platformio.ini using the provided template"
echo "  - Run 'pio run -t upload'"
echo ""
