#include <inter-com.hpp>
#include <async-core.h>
#include <interval.h>
#include <timeout.h>
#include <mac.h>
#include "device.hpp"
#include "level-sensor.h"
#include <database.h>
#include "alert-system.hpp"
#include <JSON.h>

using namespace Intervals;
using namespace Timeouts;

void setup() {
  Serial.begin(115200);



  alertSystem.begin();
  Serial2.begin(9600, SERIAL_8N1, 15, 16);
  database.begin();
  LevelSensor::begin();
  LevelSensor::onDistanceChange([](int level) {
    Device::setLevel(level);
    Serial.printf("level: %d\n", level);
  });
  Device::begin();
  interCom.on(MAIN_CALL_EVENT, []() {
    setTimeout([]() {
      interCom.flushSerial();
      uint32_t start = micros();
      if (!Device::isRegistered()) {      
        interCom.emit(REG_INIT_EVENT, mac());
      } else {
        interCom.emit(MAIN_REPLY_EVENT, Device::getData());
      }
      // Serial.println(micros() - start);
    }, NODE_ID * 50);
  });
  interCom.on(SEN_CONF_EVENT, [](String data) {
    Device::addConfiguration(data);
    Serial.println("configuration added");

  });
  interCom.on(SEN_CONF_REM_EVENT, [](JSON rawData) {
    JSON data(rawData);
    Device::removeConfiguration(data[0].toString().toInt());
    Serial.println("configuration removed");
  });
}

void loop() {
  AsyncCore::run();
  interCom.loop();
  LevelSensor::loop();
  Device::loop();  
  if (Serial.available()) {
    ESP.restart();
  }
}
