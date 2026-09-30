#pragma once

#include <Arduino.h>
#include <Wire.h>


inline void initTouch() {
  Wire.begin(I2C_SCL, I2C_SDA);
}

inline void readTouchData(uint16_t *x, uint16_t *y, uint16_t *strength, uint8_t *count) {
  // The panel firmware does not currently consume touch events. Keep this
  // small protocol probe isolated until the controller packet format is
  // verified on hardware.
  Wire.beginTransmission(TOUCH_I2C_ADDRESS);
  Wire.write(0x01);
  Wire.endTransmission();

  Wire.requestFrom(TOUCH_I2C_ADDRESS, 6);
  if (Wire.available() >= 6) {
    *count = Wire.read() & 0x0F;
    *x = (Wire.read() << 8) | Wire.read();
    *y = (Wire.read() << 8) | Wire.read();
    *strength = (Wire.read() << 8) | Wire.read();
  }
}
