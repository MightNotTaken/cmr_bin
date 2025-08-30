#ifndef HOME_SCREEN_H__
#define HOME_SCREEN_H__
#include <Arduino.h>
#include <ui.h>
#include <lvgl.h>

namespace HomeScreen {
  void setup();
  void setSignalStrength(int signal);
  void setTime(String time);
  void displayFurnace(String name, String stats, String mac, int level, int state, String material);
};

#endif