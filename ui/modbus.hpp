#pragma once
#include <Arduino.h>

#define ModbusSerial    Serial2

namespace Modbus {
  void begin();
  void readMode();
  void writeMode();
  void write(int slaveID, int reg);
  void loop();
};
  