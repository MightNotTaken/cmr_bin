#include "furnace-ctrl.hpp"
#include <database.h>

Furnace::Furnace(int slaveID, String mac, String name, String material): slaveID(slaveID), mac(mac), name(name), material(material) {
    connected = false;
    lastUpdate = millis();
    temperatureReading = new ParamReading(61000, 0.0, 3000.0);
    levelReading = new ParamReading(61000, 301.0, 30000.0);
}

void Furnace::update(int level, int temperature) {
    if (level > 30000) {
        level = 0;
    }
    this->level = level;
    this->temperature = temperature;
    this->levelReading->update(static_cast<float>(level));
    this->temperatureReading->update(static_cast<float>(temperature));
    lastUpdate = millis();
}

int Furnace::getActiveConfigState() {
    for (auto config: list) {
        if (config->isActive()) {
            return config->getState();
        }
    }
    return ALL_ALERTS_OFF;
}

bool Furnace::isConnected() {
    return this->connected;
}

int Furnace::getSlaveID() {
    return slaveID;
}


int Furnace::getTemperature() {
    return temperature;
}

int Furnace::getLevel() {
    if (level > 30000 || level < 0) {
        level = 0;
    }
    return level;
}

String Furnace::toString() {
    JSON data("[]");
    data.push_back(slaveID);
    data.push_back(mac);

    data.push_back(name.length() ? name : mac);
    data.push_back(material);
    return data.toString();
}

void Furnace::addConfiguration(JSON input, bool save) {
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

String Furnace::getConfigPath() {
    return String("/") + mac + "-fc.conf";
}

void Furnace::loadConfiguration() {
    if (database.hasFile(getConfigPath())) {
        database.readFile(getConfigPath());
        JSON data(database.payload());
        for (int i = 0; i < data.size(); i++) {
            JSON config = data[i];
            addConfiguration(config, false);
        }
    } else {
        Serial.println("No furnace configuration found.");
    }
}

void Furnace::saveConfiguration() {
    JSON data("[]");
    for (auto configuration: list) {
        data.push_back(configuration->toMinimalString());
    }
    Serial.printf("saving configuration to %s %d\n", getConfigPath().c_str(), list.size());
    Serial.println(data.toString());

    database.writeFile(getConfigPath(), data.toString());
}

void Furnace::removeConfiguration(uint32_t id, bool save) {
    auto it = std::remove_if(list.begin(), list.end(), [id](SensorConfiguration_T* configuration) {
        if (configuration->getID() == id) {
            delete configuration;
            Serial.printf("removign configuration with id: %d\n", id);
            return true;
        }
        return false;
    });
    list.erase(it, list.end());
    if (save) {
        saveConfiguration();
    }
}

String Furnace::getDisplayName() {
    if (!name.length()) {
        return mac;
    }
    return name;
}

void Furnace::loop() {
    for (auto configuration: list) {
        if (!configuration->inRange(level, temperature)) {
            if (configuration->isActive()) {
                configuration->deactivate();
                uint16_t status = ALL_ALERTS_OFF;
            }
        }
    }
    
    for (auto configuration: list) {
        if (configuration->inRange(level, temperature)) {
            if (!configuration->isActive()) {
                configuration->activate();
                uint16_t status = configuration->getState();
            }
        }
    }
}

namespace FurnaceCtrl {
    int activeIndex = 0;
    std::map<String, Furnace*> list;
    std::map<String, Furnace*>::iterator current = list.end();
    std::map<String, Furnace*>::iterator active = list.end();
    
    void begin() {
        JSON saved = getSavedList();
        Serial.println(saved.toString());
        
        for (int i=0; i<saved.size(); i++) {
            int slaveID = saved[i][0].toInt();
            String mac = saved[i][1].toString();
            String name = saved[i][2].toString();
            String material = "No Material";
            if (saved[i].size() > 3) {
                material = saved[i][3].toString();
            }
            
            Serial.println(saved.toString());
            Serial.println("furnace details: ");
            Serial.println(slaveID);
            Serial.println(mac);
            Serial.println(name);
            Serial.println(material);
            
            addFurnace(slaveID, mac, name, material);
        }
    }

    JSON getSavedList() {
        if (!database.hasFile(FURNACE_DATABASE)) {
            return "[]";
        }
        database.readFile(FURNACE_DATABASE);
        return database.payload();
    }

    int size() {
        return list.size();
    }

    void loop() {
        static uint32_t start = millis();
        for (auto& [mac, furnace]: list) {
            furnace->loop();
        }
    }
    bool addFurnace(int slaveID, String mac, String name, String material) {
        auto it = list.find(mac);
        if (it == list.end()) {
            Serial.printf("adding furnace: %s\n", mac);
            list[mac] = new Furnace(slaveID, mac, name, material);
            list[mac]->loadConfiguration();
            if (list.size() == 1) {
                current = list.begin(); // Initialize on first insert
            }
        
            return true;
        }
        return false;
    }

    void updateFurnace(String name, String mac, int slaveID) {
        auto it = list.find(mac);
        if (it == list.end()) {
            // Not found, create new Furnace and set name
            Serial.printf("creating new furnace: %s with name: %s\n", mac.c_str(), name.c_str());
            list[mac] = new Furnace(slaveID, mac, name, ""); // You can set a default or pass a proper slaveID
            list[mac]->name = name;
            list[mac]->loadConfiguration();
            if (list.size() == 1) {
                current = list.begin(); // Initialize if this is the first
            }
        } else {
            // Found, just update the name
            Serial.printf("updating furnace: %s to new name: %s\n", mac.c_str(), name.c_str());
            it->second->name = name;
        }
        save();

    }

    void updateFurnaceParams(uint32_t id, int level, int temperature) {
      for (auto& [mac, furnace]: list) {
        if (furnace->getSlaveID() == id) {
          furnace->update(level, temperature);
          Serial.println("updated furnace");
          Serial.printf("%d : %d\n", level, temperature);
          return;
        }
      }
    }


    void removeFurnace(String mac) {
        auto it = list.find(mac);
        if (it != list.end()) {
            if (it == current) {
                current = list.erase(it); // erase returns next iterator
                if (current == list.end() && !list.empty()) {
                    current = list.begin();
                }
            } else {
                delete it->second;
                list.erase(it);
            }
        }

        if (list.empty()) {
            current = list.end();
        }
    }

    Furnace* getNextFurnace() {
        if (list.empty()) return nullptr;

        if (current == list.end()) {
            current = list.begin();
        }

        Furnace* result = current->second;

        ++current;
        if (current == list.end()) {
            current = list.begin();
        }

        return result;
    }

    Furnace* getActiveFurnace() {
        if (list.empty()) return nullptr;
        
        if (active == list.end()) {
            active = list.begin();
        }
        
        Furnace* result = active->second;
        
        return result;
    }

    Furnace* activatePrevFrunace() {

        if (list.empty()) return nullptr;

        if (active == list.begin()) {
            // wrap around to last element
            active = std::prev(list.end());
            activeIndex = list.size() - 1;
        } else if (active == list.end()) {
            // if not initialized yet, go to last element
            active = std::prev(list.end());
            activeIndex = list.size() - 1;
        } else {
            // move backwards
            --active;
            --activeIndex;
        }

        Furnace* result = active->second;

        return result;
    }


    Furnace* activateNextFrunace() {
        
        if (list.empty()) return nullptr;

        if (active == list.end()) {
            active = list.begin();
            activeIndex = 0;
        }

        Furnace* result = active->second;

        ++active;
        ++activeIndex;
        if (active == list.end()) {
            active = list.begin();
            activeIndex = 0;
        }

        return result;
    }

    void clearAll() {
        for (auto& [_, ptr] : list) {
            delete ptr;
        }
        list.clear();
        current = list.end();
    }

    void save() {
        JSON furnaces("[]");
        for (auto& [mac, furnace] : list) {
            furnaces.push_back(furnace->toString());
            Serial.print("saving: ");
            Serial.println(furnace->toString());
        }
        database.writeFile(FURNACE_DATABASE, furnaces.toString());
    }

    bool has(String mac) {
        return list.find(mac) != list.end();
    }

    
    Furnace* getFurnace(String mac) {
        Serial.print("mac: ");
        Serial.println(mac);
        for (auto& [mac, furnace] : list) {
            Serial.println(furnace->toString());
        }

        auto it = list.find(mac);
        if (it != list.end()) {
            return it->second;
        }
        Serial.println("no furnace found");
        return nullptr;
    }

    
    int addConfigurationToFurnace(String rawData) {
        JSON rawDataJSON(rawData);
        String macStr = rawDataJSON[rawDataJSON.size() - 1].toString();
        Serial.printf("macStr: %s\n", macStr.c_str());
        for (auto& [mac, furnace]: list) {
            Serial.printf("mac: %s\n", mac.c_str());
            if (mac.equals(macStr)) {
                furnace->removeConfiguration(rawDataJSON[0].toInt());
                furnace->addConfiguration(rawData);
                return furnace->slaveID;
            }
        }
        return -1;
    }

    int removeConfiguration(String rawData) {        
        JSON rawDataJSON(rawData);
        String macStr = rawDataJSON[rawDataJSON.size() - 1].toString();
        for (auto& [mac, furnace]: list) {
            if (mac.equals(macStr)) {
                furnace->removeConfiguration(rawDataJSON[0].toInt());
                return furnace->slaveID;
            }
        }
        return -1;
    }

    String getMac(String name) {
        for (auto& [mac, furnace]: list) {
            if (furnace->getDisplayName() == name) {
                return mac;
            }
        }
        return "";
    }

    JSON getFurnaceOptions() {
        JSON response("[]");
        for (auto& [mac, furnace]: list) {
            response.push_back(furnace->getDisplayName());
        }
        return response;
    }

    
    void setMaterial(String mac, String material) {
        
        auto it = list.find(mac);
        if (it != list.end()) {
            it->second->material = material;
        }
        save();
    }

    String getStats() {
        return String(" (") + (activeIndex + 1) + "/" + list.size() + ")";
    }

}



