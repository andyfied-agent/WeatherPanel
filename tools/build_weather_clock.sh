#!/bin/bash
# ESP32 Weather Satellite Clock - Build and Flash Script
# This script helps compile and flash firmware to the ESP32-S3-Touch-LCD-3.49

set -e

# Configuration
PROJECT_DIR="${PROJECT_DIR:-~/opt/WeatherPanel/WeatherSatelliteImageClock}"
ARDUINO_IDE="${ARDUINO_IDE:-arduino}"
SKETCH="${SKETCH:-WeatherSatelliteImageClock}"
PORT="${PORT:-/dev/ttyUSB0}"
BAUD_RATE="${BAUD_RATE:-115200}"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Functions
print_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check prerequisites
check_prerequisites() {
    print_info "Checking prerequisites..."

    # Check for Arduino IDE
    if ! command -v $ARDUINO_IDE &> /dev/null; then
        print_warn "Arduino IDE not found in PATH"
        print_info "Make sure Arduino IDE is installed"
        print_info "Download from: https://www.arduino.cc/en/software"
        return 1
    fi

    # Check for ESP32 board support
    if [ ! -d "$HOME/ArduinoHardware/arduino-esp32" ]; then
        print_warn "ESP32 board support not found in Arduino directories"
        print_info "Install via Arduino IDE: Tools > Board > Boards Manager > esp32"
        return 1
    fi

    # Check for required libraries
    if [ ! -d "$HOME/Arduino/libraries/Arduino_GFX" ]; then
        print_warn "Arduino_GFX library not found"
        print_info "Install via Library Manager: Sketch > Include Library > Manage Libraries > Arduino GFX"
        return 1
    fi

    print_info "Prerequisites check complete"
    return 0
}

# Check project files
check_project() {
    print_info "Checking project files..."

    if [ ! -d "$PROJECT_DIR" ]; then
        print_error "Project directory not found: $PROJECT_DIR"
        return 1
    fi

    if [ ! -f "$PROJECT_DIR/$SKETCH.ino" ]; then
        print_error "Main sketch not found: $PROJECT_DIR/$SKETCH.ino"
        return 1
    fi

    # Check for Config.h
    if [ ! -f "$PROJECT_DIR/Config.h" ]; then
        print_warn "Config.h not found, creating default..."
        cat > "$PROJECT_DIR/Config.h" << 'EOF'
// Configuration for Weather Satellite Image Clock
// Target: ESP32-S3-Touch-LCD-3.49 with AXS15231B display

#ifndef CONFIG_H
#define CONFIG_H

// WiFi Configuration
#define SSID_NAME "YourWiFiNetwork"
#define SSID_PASSWORD "YourWiFiPassword"

// Timezone Configuration (Hong Kong = UTC+8)
#define GMT_OFFSET_SEC (8 * 60 * 60)

// Display Configuration
// AXS15231B with 172x640 resolution (portrait orientation)
#define GFX_BL 46  // Backlight control pin

// QSPI Display Interface Pins for ESP32-S3
#define QSPI_CS   45  // Chip Select
#define QSPI_SCK  47  // Clock
#define QSPI_D0   39  // Data 0 (MOSI)
#define QSPI_D1   48  // Data 1 (Quad Write)
#define QSPI_D2   40  // Data 2 (Quad Hold)
#define QSPI_D3   21  // Data 3 (Quad Read)

// Touch Interface (I2C) - Optional
#define TOUCH_SDA  4
#define TOUCH_SCL  8
#define TOUCH_INT  1
#define TOUCH_ADDR 0x28

// SDMMC Card Interface Pins
#define SD_CS    38
#define SD_MOSI  39
#define SD_SCK   41
#define SD_MISO  40

// Satellite Image Configuration
#include "https_nmc_cn.h"
// #include "https_met_ie.h"

// JPEG Decoder Configuration
#define JPEG_BUFFER_SIZE 4096

// Debug Configuration
#define SERIAL_BAUD 115200
#define DEBUG_OUTPUT true

// Display Settings
#define DISPLAY_WIDTH 172
#define DISPLAY_HEIGHT 640
#define DISPLAY_ROTATION 180
#define DISPLAY_IPS false

// Clock and Date Display
#define DATE_FONT u8g2_font_fub14_tf
#define TIME_FONT u8g2_font_logisoso92_tn

// Color Definitions (RGB565)
#define RGB565_BLACK       0x0000
#define RGB565_WHITE       0xFFFF
#define RGB565_RED         0xF800
#define RGB565_GREEN       0x07E0
#define RGB565_BLUE        0x001F
#define RGB565_NAVY        0x000F
#define RGB565_DARKCYAN    0x081F
#define RGB565_SILVER      0xC618
#define RGB565_GRAY        0x8430
#define RGB565_YELLOW      0xFFE0
#define RGB565_ORANGE      0xFA20
#define RGB565_PURPLE      0x8010
#define RGB565_TEAL        0x0410
#define RGB565_MAROON      0x8000

#endif // CONFIG_H
EOF
        print_info "Created default Config.h"
    fi

    print_info "Project check complete"
    return 0
}

# Check serial port
check_port() {
    print_info "Checking serial port: $PORT"

    if [ ! -e "$PORT" ]; then
        print_error "Serial port not found: $PORT"
        print_info "Available serial ports:"
        ls -la /dev/ttyUSB* /dev/ttyACM* 2>/dev/null || echo "No serial ports found"
        return 1
    fi

    # Check permissions
    if [ ! -w "$PORT" ]; then
        print_warn "Permission denied for $PORT"
        print_info "Add user to dialout group: sudo usermod -a -G dialout $USER"
        print_info "Then log out and log back in"
        return 1
    fi

    print_info "Serial port check complete"
    return 0
}

# Build the project
build() {
    print_info "Building project..."

    cd "$PROJECT_DIR"

    # Use Arduino CLI if available
    if command -v arduino-cli &> /dev/null; then
        print_info "Using Arduino CLI"
        arduino-cli compile --fqbn esp32:esp32:esp32s3 \
            --build-property "build.partitions=huge_app" \
            --build-property "build.esphome.psram=octal" \
            "$SKETCH.ino" 2>&1 | tee build.log

        if [ ${PIPESTATUS[0]} -eq 0 ]; then
            print_info "Build successful!"
            return 0
        else
            print_error "Build failed. Check build.log for details"
            return 1
        fi
    else
        print_info "Arduino CLI not found, using Arduino IDE"
        print_info "Open Arduino IDE and build manually:"
        print_info "  1. File > Open > $PROJECT_DIR/$SKETCH.ino"
        print_info "  2. Sketch > Verify/Compile"
        return 1
    fi
}

# Flash the firmware
flash() {
    print_info "Flashing firmware to $PORT..."

    cd "$PROJECT_DIR"

    # Use Arduino CLI if available
    if command -v arduino-cli &> /dev/null; then
        arduino-cli upload -p "$PORT" \
            --fqbn esp32:esp32:esp32s3 \
            --serial-speed $BAUD_RATE \
            "$SKETCH.ino" 2>&1 | tee flash.log

        if [ ${PIPESTATUS[0]} -eq 0 ]; then
            print_info "Upload successful!"
            print_info "Reset the board to run the firmware"
            return 0
        else
            print_error "Upload failed. Check flash.log for details"
            return 1
        fi
    else
        print_info "Arduino CLI not found, using Arduino IDE"
        print_info "Open Arduino IDE and flash manually:"
        print_info "  1. File > Open > $PROJECT_DIR/$SKETCH.ino"
        print_info "  2. Tools > Port > $PORT"
        print_info "  3. Tools > Board > ESP32S3 Dev Module"
        print_info "  4. Tools > Partition Scheme > Huge APP (8MB SPIFFS)"
        print_info "  5. Tools > PSRAM > Octal PSRAM"
        print_info "  6. Click Upload button"
        return 1
    fi
}

# Monitor serial output
monitor() {
    print_info "Starting serial monitor on $PORT (baud: $BAUD_RATE)..."

    if command -v screen &> /dev/null; then
        screen "$PORT" $BAUD_RATE
    elif command -v minicom &> /dev/null; then
        minicom -D "$PORT" -b $BAUD_RATE
    elif command -v cu &> /dev/null; then
        cu -p "$PORT" -s $BAUD_RATE
    else
        print_warn "No serial monitor found (screen, minicom, or cu)"
        print_info "Install one of these tools for serial monitoring"
    fi
}

# Show usage
usage() {
    cat << EOF
Usage: $0 [OPTION]

Build and flash ESP32 Weather Satellite Clock firmware

Options:
    -b, --build          Build the project
    -f, --flash          Flash the firmware
    -m, --monitor        Start serial monitor
    -p, --port PORT      Set serial port (default: $PORT)
    -c, --config         Generate Config.h with default settings
    -h, --help           Show this help message

Examples:
    $0 --build           # Build the project
    $0 --flash           # Flash to default port
    $0 -p /dev/ttyUSB1   # Use different port
    $0 --build --flash   # Build and flash

Environment Variables:
    PROJECT_DIR     Project directory (default: ~/opt/WeatherPanel/WeatherSatelliteImageClock)
    ARDUINO_IDE     Arduino IDE command (default: arduino)
    PORT            Serial port (default: /dev/ttyUSB0)
    BAUD_RATE       Serial baud rate (default: 115200)

EOF
}

# Parse arguments
BUILD=false
FLASH=false
MONITOR=false
GENERATE_CONFIG=false

while [[ $# -gt 0 ]]; do
    case $1 in
        -b|--build)
            BUILD=true
            shift
            ;;
        -f|--flash)
            FLASH=true
            shift
            ;;
        -m|--monitor)
            MONITOR=true
            shift
            ;;
        -p|--port)
            PORT="$2"
            shift 2
            ;;
        -c|--config)
            GENERATE_CONFIG=true
            shift
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            usage
            exit 1
            ;;
    esac
done

# Main execution
print_info "ESP32 Weather Satellite Clock Build Script"
print_info "=========================================="

if $GENERATE_CONFIG; then
    # Generate Config.h if requested
    if [ -f "$PROJECT_DIR/Config.h" ]; then
        print_warn "Config.h already exists. Overwrite? (y/n)"
        read -r response
        if [[ "$response" =~ ^[Yy]$ ]]; then
            cat > "$PROJECT_DIR/Config.h" << 'EOF'
// Configuration for Weather Satellite Image Clock
// Target: ESP32-S3-Touch-LCD-3.49 with AXS15231B display

#ifndef CONFIG_H
#define CONFIG_H

// WiFi Configuration
#define SSID_NAME "YourWiFiNetwork"
#define SSID_PASSWORD "YourWiFiPassword"

// Timezone Configuration (Hong Kong = UTC+8)
#define GMT_OFFSET_SEC (8 * 60 * 60)

// Display Configuration
// AXS15231B with 172x640 resolution (portrait orientation)
#define GFX_BL 46  // Backlight control pin

// QSPI Display Interface Pins for ESP32-S3
#define QSPI_CS   45  // Chip Select
#define QSPI_SCK  47  // Clock
#define QSPI_D0   39  // Data 0 (MOSI)
#define QSPI_D1   48  // Data 1 (Quad Write)
#define QSPI_D2   40  // Data 2 (Quad Hold)
#define QSPI_D3   21  // Data 3 (Quad Read)

// Touch Interface (I2C) - Optional
#define TOUCH_SDA  4
#define TOUCH_SCL  8
#define TOUCH_INT  1
#define TOUCH_ADDR 0x28

// SDMMC Card Interface Pins
#define SD_CS    38
#define SD_MOSI  39
#define SD_SCK   41
#define SD_MISO  40

// Satellite Image Configuration
#include "https_nmc_cn.h"
// #include "https_met_ie.h"

// JPEG Decoder Configuration
#define JPEG_BUFFER_SIZE 4096

// Debug Configuration
#define SERIAL_BAUD 115200
#define DEBUG_OUTPUT true

// Display Settings
#define DISPLAY_WIDTH 172
#define DISPLAY_HEIGHT 640
#define DISPLAY_ROTATION 180
#define DISPLAY_IPS false

// Clock and Date Display
#define DATE_FONT u8g2_font_fub14_tf
#define TIME_FONT u8g2_font_logisoso92_tn

// Color Definitions (RGB565)
#define RGB565_BLACK       0x0000
#define RGB565_WHITE       0xFFFF
#define RGB565_RED         0xF800
#define RGB565_GREEN       0x07E0
#define RGB565_BLUE        0x001F
#define RGB565_NAVY        0x000F
#define RGB565_DARKCYAN    0x081F
#define RGB565_SILVER      0xC618
#define RGB565_GRAY        0x8430
#define RGB565_YELLOW      0xFFE0
#define RGB565_ORANGE      0xFA20
#define RGB565_PURPLE      0x8010
#define RGB565_TEAL        0x0410
#define RGB565_MAROON      0x8000

#endif // CONFIG_H
EOF
        print_info "Config.h generated"
    else
        echo "Config.h already exists, skipping"
    fi
    exit 0
fi

# Run checks
check_prerequisites || exit 1
check_project || exit 1

if $BUILD; then
    build || exit 1
fi

if $FLASH; then
    check_port || exit 1
    flash || exit 1
fi

if $MONITOR; then
    check_port || exit 1
    monitor
fi

if ! $BUILD && ! $FLASH && ! $MONITOR; then
    print_info "No action specified. Use --help for usage information"
    usage
fi

print_info "Done!"
