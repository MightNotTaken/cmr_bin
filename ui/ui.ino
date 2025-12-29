#include "display.hpp"
#include "splash.hpp"
#include "home_screen.hpp"
#include <async-core.h>
#include <interval.h>
#include "auth.hpp"
#include "edit_material.hpp"
#include <database.h>
#include "cellular.hpp"
#include "config.hpp"
#include <lvgl.h>
#include <ui.h>
#include <inter-com.hpp>
#include "data-source.hpp"
#include <clock.h>
#include <mac.h>
#include "furnace-ctrl.hpp"
#include "OTA.hpp"

#define MAIN_CALL_EVENT      "up"

using namespace Intervals;
using namespace Configuration::InterCom;

void setupInterCom();
void setupDataSourceTriggers();
void runDataRoutine();
void runSecretDataRoutine();

void setup() {
    Serial.begin(115200);
    database.begin();
    database.listDir();
    
    Display::setup();
    HomeScreen::setup();
    Auth::setup();
    EditMaterial::setup();
    Configuration::begin();
    FurnaceCtrl::begin();
    Splash::splash();
    
    Cellular::begin();
    
    EditMaterial::loadMaterials();
    EditMaterial::setFurnaces(FurnaceCtrl::getFurnaceOptions());
    
    setupInterCom();
    setupUIUpdators();
    setupDataSourceTriggers();
    if (!ota.beginAP("CMR-OTIS", "OTIS@1234", true)) {
        Serial.println("Failed to start AP!");
    } else {
        Serial.print("AP IP: "); Serial.println(WiFi.softAPIP());
        Serial.println("Open http://192.168.4.1/ to upload firmware (.bin)");
    }
}

void loop() {
    Display::loop();
    AsyncCore::run();
    interCom.loop();
    FurnaceCtrl::loop();
    Cellular::loop();
    dataSource.loop();
    runDataRoutine();
    runSecretDataRoutine();
    ota.loop();
}

void runSecretDataRoutine() {
    static uint32_t start = millis();
    static uint32_t lastAttempt = millis();
    if (millis() - start > 1000) {
        start = millis();        
        for (auto& [mac, furnace]: FurnaceCtrl::list) {
            int latest = furnace->levelReading->readLatest();
            database.appendFile(furnace->getSecretFile(), String(latest)+ ',');
            if (millis() - lastAttempt > MINUTES(5) + SECONDS(10)) {
                lastAttempt = millis();
                String millistr = String("\n") + millis();
                database.appendFile(furnace->getSecretFile(), millistr);
                if (Cellular::internetConnected) {
                    Cellular::cellularState = CellularState::CELL_FLUSH_SECRET_LOGS;
                }
            }
        }
    }
}

void runDataRoutine() {
    static uint32_t lastAttempt = 0;
    if (millis() - lastAttempt > 60000) {
        lastAttempt = millis();
        for (auto& [mac, furnace]: FurnaceCtrl::list) {
            if (furnace->isActive()) {
                JSON data("[]");
                data.push_back(furnace->mac);
                data.push_back(static_cast<uint32_t>(furnace->levelReading->consolidate()));
                data.push_back(static_cast<uint32_t>(furnace->temperatureReading->consolidate()));
                furnace->levelReading->reset();
                furnace->temperatureReading->reset();
                data.push_back(systemClock.getCurrentTime().epoch() - 330 * 60);
                if (Cellular::internetConnected && Cellular::emit("cmr:data", data.toString())) {
                    Serial.println("data sent");
                } else {
                    Serial.println("unable to log data");
                    Serial.println("Saving for future");
                    String file = furnace->getLogFile();
                    if (!database.hasFile(file)) {
                        database.createFile(file);
                    }
                    String strData = data.toString();
                    strData.replace("[", "");
                    strData.replace("]", "");
                    database.writeFile(file, strData + "|", FILE_APPEND);
                    database.readFile(file);
                    Serial.println(database.payload());
                    Cellular::cellularState = CellularState::CELL_WAIT_TURN_ON;
                }
            }
        }
    }
}

void setupUIUpdators() {
    setInterval([]() {
        static int last = -1;
        if (Cellular::internetConnected) {
            int strength = Cellular::getNetworkStrength();
            if (strength != last) {
                HomeScreen::setSignalStrength(map(strength, 0, 31, 0, 100));
            }
            last = strength;
        } else {
            HomeScreen::setSignalStrength(map(0, 0, 31, 0, 100));
        }
    }, 5000);

    setInterval([]() {
        auto furnace = FurnaceCtrl::getActiveFurnace();
        if (furnace) {
            Serial.printf("active: %s %d\n", furnace->name.c_str(), furnace->level);
            HomeScreen::displayFurnace(furnace->name, FurnaceCtrl::getStats(), furnace->mac, furnace->level, furnace->getActiveConfigState(), furnace->material);
        } else {
            HomeScreen::displayFurnace("No Furnaces", "(0/0)", "--", 0, ALL_ALERTS_OFF, "--");
        }
    }, 1000);


    setInterval([]() {
        HomeScreen::setTime(systemClock.getCurrentTime().toString());
    }, 1000);
}

void setupInterCom() {

    Serial2.begin(9600, SERIAL_8N1, Configuration::InterCom::rx, Configuration::InterCom::tx);

    setInterval([]() {
        interCom.emit(MAIN_CALL_EVENT, "");
    }, 1000);
    
    for (int i=1; i<=20; i++) {
        interCom.on(String("u") + i, [i](String data) {
            Serial.printf("%s received from %d\n", data.c_str(), i);
            int level;
            int temperature;
            sscanf(data.c_str(), "%d,%d", &level, &temperature);
            FurnaceCtrl::updateFurnaceParams(i, level, temperature);
        });
        
        interCom.on(String("r") + i, [i](String mac) {
            Serial.print("registered ");
            Serial.println(mac);
            interCom.emit(String("c") + i, "");
            FurnaceCtrl::addFurnace(i, mac);
            FurnaceCtrl::save();
            EditMaterial::setFurnaces(FurnaceCtrl::getFurnaceOptions());
        });
    }
}


void setupDataSourceTriggers() {

    dataSource.on("utc-now", [](String data) {
        Serial.println(data);
        systemClock.setCurrentTime(data.toInt() + (5.5) * 60 * 60);
    });
    
    dataSource.on("add-furnace", [](String rawdata) {
        Cellular::emit(mac() + ":add-furnace", "");
        JSON data(rawdata);
        Serial.println(rawdata);
        String name = data[0].toString();
        String uuid = data[1].toString();
        Serial.printf("%s: %s\n", name.c_str(), uuid.c_str());
        FurnaceCtrl::updateFurnace(name, uuid);
        EditMaterial::setFurnaces(FurnaceCtrl::getFurnaceOptions());
    });

    dataSource.on("remove-furnace", [](String rawdata) {
        Cellular::emit(mac() + ":remove-furnace", "");
        JSON data(rawdata);
        Serial.println(rawdata);
        String uuid = data[0].toString();
        Serial.println(uuid);
        FurnaceCtrl::removeFurnace(uuid);
    });

    dataSource.on("add-config", [](String data) {
        Serial.println(data);
        Cellular::emit(mac() + ":add-config", "");
        int slaveID = FurnaceCtrl::addConfigurationToFurnace(data);
        if (slaveID > -1) {
            interCom.emit(String("sa") + slaveID, data);
        }
    });

    dataSource.on("add-material", [](String data) {
        Serial.println(data);
        EditMaterial::addMaterial(data);
        Cellular::emit(mac() + ":add-material", "");
    });
    
    dataSource.on("rem-material", [](String data) {
        Serial.println(data);
        EditMaterial::removeMaterial(data);
        Cellular::emit(mac() + ":rem-material", "");
    });
    
    dataSource.on("remove-config", [](String data) {
        Cellular::emit(mac() + ":remove-config", "");
        int slaveID = FurnaceCtrl::removeConfiguration(data);
        if (slaveID > -1) {
            interCom.emit(String("sr") + slaveID, data);
        }
    });
}