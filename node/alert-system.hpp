#ifndef ALERT_SYSTEM_H__
#define ALERT_SYSTEM_H__
#include <Arduino.h>

#define SOUND_PIN       35
#define YELLOW_PIN      47
#define RED_PIN         48
#define GREEN_PIN       21

#define ALL_ALERTS_OFF   (0)
#define SOUND_ON         (1 << 0)
#define RED_ON           (1 << 1)
#define GREEN_ON         (1 << 2)
#define YELLOW_ON        (1 << 3)
#define ALL_COLORS_ON    (RED_ON|GREEN_ON|YELLOW_ON)


typedef uint32_t AlertState_T;

class AlertSystem {
    AlertState_T state;
public:
    void begin();
    void shutdown();
    void activate(AlertState_T state);
    void show();
};

extern AlertSystem alertSystem;
#endif
