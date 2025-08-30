#pragma once
#include <Arduino.h>
#include <functional>
namespace LevelSensor {
  void begin();
  void loop();
  int getDistance();
  void onDistanceChange(std::function<void(int)> cb);
};