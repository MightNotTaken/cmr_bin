#ifndef TOUCH_H
#define TOUCH_H

#include <Arduino.h>
#include <Wire.h>

#define FT6X36_ADDR    0x38
#define TOUCH_RESET    8
#define TOUCH_IRQ      21

class TouchController {
public:
  TouchController();
  void loop();
  void begin();
  bool getTouch(uint16_t* x, uint16_t* y);

private:
  bool readTouchData(uint8_t* buffer);
  uint8_t readTouchPoints();
  uint16_t lastX, lastY;
  bool touchAvailable;
};

extern TouchController touchCtrl;

#endif