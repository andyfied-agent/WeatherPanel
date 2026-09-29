#pragma once

// This integration targets the Waveshare ESP32-S3-Touch-LCD-3.49 V2 board.
// Keep all board-specific values here so the sketch and documentation cannot
// silently drift apart.
#define WEATHERPANEL_BOARD_VERSION 2

// Network Configuration. Replace these placeholders locally before flashing.
const char *SSID_NAME = "YourAP";
const char *SSID_PASSWORD = "PleaseInputYourPasswordHere";
const long gmtOffset_sec = 8 * 60 * 60; // GMT+8 (Hong Kong)

// Display Configuration for ESP32-S3-Touch-LCD-3.49 V2
#define DISP_WIDTH 172
#define DISP_HEIGHT 640
#define DISP_ROTATION 0  // 0 = portrait (default), 1=90°, 2=180°, 3=270°

// QSPI pins from the V2 board configuration.
#define QSPI_CS 45
#define QSPI_SCK 47
#define QSPI_D0 39
#define QSPI_D1 48
#define QSPI_D2 40
#define QSPI_D3 21

// I2C touch controller.
#define I2C_SCL 8
#define I2C_SDA 4
#define TOUCH_I2C_ADDRESS 0x38
#define TOUCH_INT_GPIO 1

// The V2 board shares GPIO 46 for display reset and backlight control.
#define GFX_BL 46
#define DISPLAY_RST 46
#define DISP_IPS false

// SD_MMC uses (clock, command, data0) in 1-bit mode.
#define SDMMC_CS 38
#define SD_MMC_CLK 41
#define SD_MMC_CMD 39
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
