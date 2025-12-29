
#pragma once
#include <Arduino.h>
#include <functional>

// #define NON_MODBUS_LUEZE
#define MODBUS_LUEZE

namespace LevelSensor {
  void begin();
  void loop();
  int getDistance();
  void onDistanceChange(std::function<void(int)> cb);
};