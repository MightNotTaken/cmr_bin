// alarm.h
#ifndef ALARM_H
#define ALARM_H

#include "Time_T.h"
#include <Arduino.h>
#include <list>
#include <memory>
#include <console.h>
#include <definitions.h>
#include <expire.h>
#include <timeout.h>
#include <repeater.h>
#include <interval.h>
#include <event-handler.h>

using namespace Timeouts;
using namespace Repeaters;
using namespace Intervals;

class Alarm: public Expire, public EventHandler {
    Time_T startTime;
    uint32_t repeatInterval;
    int repeatCount;
    Timeout* initialWait;
    Repeater* repeater;
public:
    
    Alarm** autoClear;
    Alarm(Time_T start, int32_t repeatAfter = 0, int howMany = 1, Alarm** autoClear = nullptr);
    void schedule();
    void unschedule();
    ~Alarm();
};

namespace Alarms {
    Alarm* setAlarm(Time_T start, int32_t repeatAfter = 0, int howMany = 1, Alarm** autoClear = nullptr);
    void clearAlarm(Alarm*& alarm);
    void clearAllAlarms();
    void begin();
}

#endif // ALARM_H