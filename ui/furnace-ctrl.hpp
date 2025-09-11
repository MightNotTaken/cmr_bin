#pragma once
#include <Arduino.h>
#include <map>
#include <JSON.h>
#include "sensor-configuration.h"
#include "param-reading.hpp"


#define FURNACE_DATABASE        "/furnace.json"

class Furnace {
public:
    String mac;
    uint32_t lastUpdate;
    int temperature;
    int level;
    uint16_t registers[2];
    std::vector<SensorConfiguration_T*> list;
    bool connected;
    int slaveID;
    String name;
    uint32_t id;
    String material;

    ParamReading* temperatureReading;
    ParamReading* levelReading;
    Furnace(int slaveID, String mac, String name="", String material = "No Material");
    int getActiveConfigState();
    void update(int level, int temperature);
    int getTemperature();
    int getLevel();
    String toString();
    int getSlaveID();
    String getDisplayName();
    void addConfiguration(JSON input, bool save = true);
    void removeConfiguration(uint32_t id, bool save = true);
    void saveConfiguration();
    String getConfigPath();
    void loadConfiguration();
    void loop();
    bool isConnected();
    uint32_t getID() {
        return id;
    };
    bool isActive() {
        return millis() - lastUpdate < 60000;
    }
    String getLogFile() {
        return String("/logf-") + this->mac + ".txt";
    }
};

namespace FurnaceCtrl {
    extern std::map<String, Furnace*> list;
    void loop();
    bool addFurnace(int slaveID, String mac, String name = "", String material = "No Material");
    void removeFurnace(String mac);
    Furnace* getNextFurnace();
    Furnace* getActiveFurnace();
    void save();
    int size();
    void begin();
    JSON getSavedList();
    int getSlaveID();
    Furnace* activateNextFrunace();
    Furnace* activatePrevFrunace();
    bool has(String mac);
    Furnace* getFurnace(String mac);
    int addConfigurationToFurnace(String rawData);
    int removeConfiguration(String id);
    void updateFurnaceParams(uint32_t id, int level, int temperature);
    void updateFurnace(String name, String mac, int slaveID = -1);
    JSON getFurnaceOptions();
    void setMaterial(String mac, String material);
    String getMac(String name);
    String getStats();
};