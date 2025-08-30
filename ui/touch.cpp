#include "touch.h"

#define I2C_SDA 48
#define I2C_SCL 35

TouchController::TouchController() : lastX(0), lastY(0), touchAvailable(false) {
}

void TouchController::begin() {
  pinMode(TOUCH_RESET, OUTPUT);
  digitalWrite(TOUCH_RESET, HIGH);
  Wire.begin(I2C_SDA, I2C_SCL);
}

void TouchController::loop() {
  uint8_t touch_points = readTouchPoints();
  if (touch_points > 0) {
    uint8_t buffer[6];
    if (readTouchData(buffer)) {
      lastX = ((buffer[0] & 0x0F) << 8) | buffer[1]; // P1_XH and P1_XL
      lastY = ((buffer[2] & 0x0F) << 8) | buffer[3]; // P1_YH and P1_YL
      touchAvailable = true;
      Serial.printf("(%d, %d)\n", lastX, lastY);
    }
  }
}

bool TouchController::getTouch(uint16_t* x, uint16_t* y) {
  if (touchAvailable) {
    touchAvailable = false;
    *x = lastX;
    *y = lastY;
    return true;
  }
  return false;
}

uint8_t TouchController::readTouchPoints() {
  uint8_t data;

  Wire.beginTransmission(FT6X36_ADDR);
  Wire.write(0x02); // TD_STATUS register
  if (Wire.endTransmission() != 0) {
    return 0;
  }

  Wire.requestFrom(FT6X36_ADDR, 1);
  if (Wire.available()) {
    data = Wire.read();
    return data & 0x0F; // Mask to get number of touch points (0-2)
  }
  return 0;
}

bool TouchController::readTouchData(uint8_t* buffer) {
  Wire.beginTransmission(FT6X36_ADDR);
  Wire.write(0x03); // Start at P1_XH
  if (Wire.endTransmission() != 0) {
    return false;
  }

  Wire.requestFrom(FT6X36_ADDR, 6); // Read 6 bytes (P1_XH to P1_YL)
  if (Wire.available() == 6) {
    for (int i = 0; i < 6; i++) {
      buffer[i] = Wire.read();
    }
    return true;
  }
  return false;
}

TouchController touchCtrl;