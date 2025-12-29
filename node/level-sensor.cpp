#include "level-sensor.h"

#define modbusSerial   Serial1

namespace LevelSensor {
  int distance = 0;
  int lastDistance = 0;
  bool busy = false;
  uint32_t lastTimestamp = millis();
  int byteIndex = 0;
  uint8_t response[50];
  std::function<void(int)> callback;


  void begin() {
    modbusSerial.begin(9600, SERIAL_8N1, 4, 5);
  }

  bool isBusy() {
    if (millis() - lastTimestamp > 300) {
      lastTimestamp = millis();
      busy = false;
    }
    return busy;
  }

#if defined(MODBUS_LUEZE)
    
  void read_sensor() {
    char buffer[10];
    int bufferIndex = 0;
    
    memset(buffer, 0, 10);

    while (modbusSerial.available()) {
      char ch =  modbusSerial.read();
      if (ch == '\n') {
        break;
      }
      buffer[bufferIndex] = ch;
      bufferIndex ++;
      if (bufferIndex >= 10) {
        break;
      }
      delay(4);
    }
    if (bufferIndex > 0) {
      sscanf(buffer, "%d", &distance);
    }
  }

#elif defined(NON_MODBUS_LUEZE)

  void read_sensor() {
    if (!isBusy()) {
      uint8_t bytes[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x02, 0xC4, 0x0B};
      modbusSerial.write(bytes, 8);
      modbusSerial.flush();
      lastTimestamp = millis();
      byteIndex = 0;
      memset(response, 0, 50);
      busy = true;
    }
    if (modbusSerial.available()) {
      while (modbusSerial.available()) {
        response[byteIndex] = modbusSerial.read();
        byteIndex ++;
        byteIndex %= 50; 
      }
    }
    if (byteIndex >= 8) {
      int value = response[3]<<8;
      value |= response[4];
      distance = map(value, 0, 4096, 300, 6000);
      busy = false;
      lastTimestamp = millis();
    }
  }
#endif

  void loop() {
    read_sensor();
    if (distance != lastDistance) {
      Serial.println(distance);
      LevelSensor::callback(distance);
    }
    lastDistance = distance;
  }


  int getDistance() {
    return distance;
  }

  
  void onDistanceChange(std::function<void(int)> cb) {
    LevelSensor::callback = cb;
  }
};
