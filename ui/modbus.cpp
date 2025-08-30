#include "modbus.hpp"
#include "config.hpp"
namespace Modbus {
  void begin() {
    ModbusSerial.begin(9600, SERIAL_8N1, Configuration::InterCom::rx, Configuration::InterCom::tx);
    pinMode(Configuration::InterCom::de_re, OUTPUT);
  }


  void readMode() {
    delay(5);
    digitalWrite(Configuration::InterCom::de_re, LOW);
  }

  void writeMode() {
    digitalWrite(Configuration::InterCom::de_re, HIGH);
    delay(5);
  }
  
  void write(int slaveID, int reg) {
    writeMode();
    ModbusSerial.printf("~%d_%d|", slaveID, reg);
    ModbusSerial.flush();
    readMode();
  }

  void loop() {
    if (ModbusSerial.available()) {
      Serial.println("__________________________________________________________________________________________________________________________________________");
      while (ModbusSerial.available()) {
        Serial.write((char)ModbusSerial.read());
        delay(3);
      }
      Serial.println("__________________________________________________________________________________________________________________________________________");
    }
    static uint32_t start = millis();
    if (millis() - start > 1000) {
      start = millis();
      write(1, 12);
      Serial.println("writing command");
    }
  }
};