#ifndef SENSOR_CONFIGURATION_H__
#define SENSOR_CONFIGURATION_H__
#include <vector>
#include <Arduino.h>

#define ALL_ALERTS_OFF   (0)
#define SOUND_ON         (1 << 0)
#define RED_ON           (1 << 1)
#define GREEN_ON         (1 << 2)
#define YELLOW_ON        (1 << 3)
#define ALL_COLORS_ON    (RED_ON|GREEN_ON|YELLOW_ON)

enum Color {
  NO_COLOR              = 0,
  GREEN                 = 1,
  YELLOW                = 2,
  RED                   = 3,
  RED_GREEN_YELLOW      = 4
};

enum Sound {
  NO_SOUND         = 0,
  BEEP_SOUND       = 1
};

enum RangeType {
  LEVEL        = 0,
  TEMPERATURE  = 1
};

class SensorConfiguration_T {
public:
    uint32_t id;
    RangeType type;
    int max;
    int min;
    Sound sound;
    Color color;
    uint32_t sensorUUID;
    bool active;
    String rawString;
    SensorConfiguration_T(
        uint32_t id,
        RangeType type,
        int max,
        int min,
        Sound sound,
        Color color,
        uint32_t sensorUUID,
        String rawString = ""
    );
    void update(
        uint32_t id,
        RangeType type,
        int max,
        int min,
        Sound sound,
        Color color,
        uint32_t sensorUUID,
        String rawString = ""
    );
    uint32_t getID();
    bool inRange(int level, int temperature);
    bool isActive();
    void activate();
    void deactivate();
    void activateSound();
    void activateColor();
    uint16_t getState();
    String toString();
    String toMinimalString();
    ~SensorConfiguration_T();
};

#endif
