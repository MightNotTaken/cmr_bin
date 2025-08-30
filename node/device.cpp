#include "device.hpp"
#include <inter-com.hpp>
#include <mac.h>
#include <vector>
#include "sensor-configuration.h"
#include <database.h>

namespace Device {
  int level = 0;
  int temperature = 0;
  bool registered = false;
  std::vector<SensorConfiguration_T*> list;

  void begin() {
    interCom.on(REG_CONF_EVENT, []() {
      Serial.println("registration completed");
      confirmRegistration();
    });
    loadConfiguration();
  }

  bool isRegistered() {
    return registered;
  }

  void confirmRegistration() {
    registered = true;
  }

  void setLevel(int level) {
    Device::level = level;
  }
  void setTemp(int temperature) {
    Device::temperature = temperature;
  }

  String getData() {
    return String(level) + "," + temperature;
  }

  
  void addConfiguration(JSON input, bool save) {
      uint32_t id = input[0].toInt();
      RangeType type = static_cast<RangeType>(input[1].toInt());
      int max = input[2].toInt();
      int min = input[3].toInt();
      Sound sound = static_cast<Sound>(input[4].toInt());
      Color color = static_cast<Color>(input[5].toInt());
      uint32_t sensorUUID = input[6].toInt();

      for (auto config: list) {
        if (config->id == id) {
          config->update(id, type, max, min, sound, color, sensorUUID, input.toString());
          if (save) {
              saveConfiguration();
          }
          return;
        }
      }
      SensorConfiguration_T* configuration = new SensorConfiguration_T(id, type, max, min, sound, color, sensorUUID, input.toString());
      list.push_back(configuration);
      Serial.println("adding new configuration to the system");
      Serial.println("here are the specifications");
      Serial.println(configuration->toString());
      Serial.print("list size:");
      Serial.println(list.size());

      for (auto config: list) {
          Serial.println(config->toString());
      }
      if (save) {
          saveConfiguration();
      }
  }

  String getConfigPath() {
      return String("/") + mac() + "-fc.conf";
  }

  void loadConfiguration() {
    String path = getConfigPath();
      if (database.hasFile(path)) {
        
          database.readFile(path);
          JSON data(database.payload());
          for (int i = 0; i < data.size(); i++) {
              JSON config = data[i];

              addConfiguration(config, false);
          }
      } else {
          Serial.println("No furnace configuration found.");
      }
      
  }

  void saveConfiguration() {
      JSON data("[]");
      for (auto configuration: list) {
          data.push_back(configuration->toMinimalString());
      }
      Serial.print("saving configuration to ");
      Serial.print(getConfigPath() + " ");
      Serial.println(list.size());
      Serial.println(data.toString());

      database.writeFile(getConfigPath(), data.toString());
  }

  void removeConfiguration(uint32_t id, bool save) {

      auto it = std::remove_if(list.begin(), list.end(), [id](SensorConfiguration_T* configuration) {
          if (configuration->getID() == id) {
              delete configuration;
              return true;
          }
          return false;
      });
      list.erase(it, list.end());
      if (save) {
          saveConfiguration();
      }
  }

  void loop() {

    for (auto configuration: list) {
        if (!configuration->inRange(level, temperature)) {
            if (configuration->isActive()) {
                configuration->deactivate();
            }
        }
    }
    
    for (auto configuration: list) {
        if (configuration->inRange(level, temperature)) {
            if (!configuration->isActive()) {
                configuration->activate();
            }
        }
    }
  }


}