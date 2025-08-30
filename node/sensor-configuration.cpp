#include "sensor-configuration.h"
#include <console.h>
#include "JSON.h"

SensorConfiguration_T::SensorConfiguration_T(uint32_t id, RangeType type, int max, int min, Sound sound, Color color, uint32_t sensorUUID, String rawString)
    : id(id), type(type), max(max), min(min), active(false), sound(sound), color(color), sensorUUID(sensorUUID), rawString(rawString) {}

uint32_t SensorConfiguration_T::getID() {
    return id;
}

String SensorConfiguration_T::toString() {
    JSON config;
    config["id"] = id;
    config["type"] = static_cast<int>(type) == RangeType::LEVEL ? "LEVEL" : "TEMPERATURE";
    config["max"] = max;
    config["min"] = min;
    config["sound"] = static_cast<int>(sound) == Sound::NO_SOUND ? "NO_SOUND" : "BEEP_SOUND";
    config["color"] = static_cast<int>(color) == Color::NO_COLOR ? "NO_COLOR" :
                      static_cast<int>(color) == Color::GREEN ? "GREEN" :
                      static_cast<int>(color) == Color::YELLOW ? "YELLOW" :
                      static_cast<int>(color) == Color::RED ? "RED" :
                      static_cast<int>(color) == Color::RED_GREEN_YELLOW ? "RED_GREEN_YELLOW" : "UNKNOWN";
    config["sensorUUID"] = sensorUUID;
    config["active"] = active;
    return config.toString();
}


bool SensorConfiguration_T::inRange(int level, int temperature) {
    if (this->type == RangeType::LEVEL) {
        return level >= min && level < max;
    } else {
        return temperature >= min && temperature < max;
    }
}

bool SensorConfiguration_T::isActive() {
    return active;
}

void SensorConfiguration_T::activateSound() {
    switch (sound) {
        case Sound::BEEP_SOUND:  {

        } break;
        case Sound::NO_SOUND: {

        } break;
    }
}


String SensorConfiguration_T::toMinimalString() {
    return rawString;
}

uint16_t SensorConfiguration_T::getState() {
    int state = ALL_ALERTS_OFF;
    switch (color) {
        case Color::GREEN: state |= GREEN_ON; break;
        case Color::YELLOW: state |= YELLOW_ON; break;
        case Color::RED: state |= RED_ON; break;
        case Color::RED_GREEN_YELLOW: state |= ALL_COLORS_ON; break;
    }
    switch (sound) {
        case Sound::BEEP_SOUND: state |= SOUND_ON;
            break;
    }
    return state;
}
void SensorConfiguration_T::update(uint32_t id, RangeType type, int max, int min, Sound sound, Color color, uint32_t sensorUUID, String rawString) {
    this->id         = id;
    this->type       = type;
    this->max        = max;
    this->min        = min;
    this->sound      = sound;
    this->color      = color;
    this->sensorUUID = sensorUUID;
    this->rawString  = rawString;
}

void SensorConfiguration_T::activate() {
    Serial.println(String("Activating sensor configuration with ID: " + String(id)).c_str());
    active = true;
    alertSystem.activate(getState());

}

void SensorConfiguration_T::deactivate() {
    Serial.println(String("Deactivating sensor configuration with ID: " + String(id)).c_str());
    active = false;
    alertSystem.shutdown();
}

SensorConfiguration_T::~SensorConfiguration_T() {
    deactivate();
}