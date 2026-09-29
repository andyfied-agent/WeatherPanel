/**
 * WeatherPanel for ESP32-S3-Touch-LCD-3.49
 *
 * This sketch adapts the original WeatherPanelHKO for the 3.49" display
 * with AXS15231B driver, QSPI interface, and I2C touch support.
 *
 * Display: 172x640 pixels (portrait orientation)
 * Driver: AXS15231B via QSPI (V2 GPIO 45, 47, 39, 48, 40, 21)
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
    DISPLAY_RST,
    DISP_ROTATION,
    DISP_IPS,
    DISP_WIDTH, // width (172)
    DISP_HEIGHT,// height (640)
    0, 0, 0, 0, // col/row offsets
    axs15231b_180640_init_operations,
    sizeof(axs15231b_180640_init_operations)
);


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

  initTouch();

#ifdef GFX_BL
  pinMode(GFX_BL, OUTPUT);
  digitalWrite(GFX_BL, HIGH);  // Turn on backlight
#endif

  // Initialize WiFi
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(SSID_NAME, SSID_PASSWORD);

  // Initialize SD/MMC
  SD_MMC.setPins(SD_MMC_CLK, SD_MMC_CMD, SD_MMC_D0);
  if (!SD_MMC.begin("/root", true /* mode1bit */, false /* format_if_mount_failed */, SDMMC_FREQ_HIGHSPEED)) {
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
  configTime(0, 0, "pool.ntp.org");
  Serial.print("Waiting for NTP time sync: ");
  time_t nowSecs = time(nullptr);
  while (nowSecs < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    yield();
    nowSecs = time(nullptr);
  }
  Serial.println();
  struct tm timeinfo;
  gmtime_r(&nowSecs, &timeinfo);
  Serial.print("Current time: ");
  char buf[26];
  Serial.println(asctime_r(&timeinfo, buf));

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
