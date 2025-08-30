#include "alarm.h"
#include <console.h>
#include <clock.h>

Alarm::Alarm(Time_T start, int32_t repeatAfter, int howMany, Alarm** autoClear)
:   startTime(start),
    repeatInterval(repeatAfter),
    repeatCount(howMany),
    autoClear(autoClear),
    initialWait(nullptr),
    repeater(nullptr) {
    schedule();
}

void Alarm::schedule() {
    uint32_t initialWaitingTime;
    if (repeatInterval < SECONDS(1)) {
        repeatInterval = SECONDS(1);
    }
    if (repeatCount < 0) {
        repeatCount = 0;
    }
    if (systemClock.getCurrentTime() < startTime) {
        initialWaitingTime = SECONDS((startTime - systemClock.getCurrentTime()).epoch());
    } else {
        uint32_t repeatSize = repeatInterval / 1000;
        uint32_t currentInstant = systemClock.getCurrentTime().epoch();
        uint32_t endInstant = (startTime + repeatCount * repeatSize).epoch();
        if (currentInstant >= endInstant) {
            this->expire();
            return;
        } else {
            if (repeatSize > 0) {
                initialWaitingTime = SECONDS((endInstant - currentInstant) % repeatSize);
                repeatCount = (endInstant - currentInstant) / repeatSize;
            }
        }
    }

    console.log("alarm will trigger after", initialWaitingTime, "milliseconds");
    initialWait = setTimeout([this]() {
        console.log("Alarm triggered", systemClock.getCurrentTime());
        this->call("trigger");
        if (repeatCount - 1 >= 0) {
            repeater = setRepeater([this]() {
                console.log("Alarm triggered", systemClock.getCurrentTime());
                this->call("trigger");
            }, repeatInterval, repeatCount - 1);
            repeater->onExpire([this]() {
                this->expire();
            });
        }
    }, initialWaitingTime);
}

void Alarm::unschedule() {
    clearTimeout(initialWait);
    clearRepeater(repeater);
}

Alarm::~Alarm() {
    unschedule();
    if (autoClear) {
        *autoClear = nullptr;
    }
    console.log("alarm erased");
}

namespace Alarms {
    static std::list<std::unique_ptr<Alarm>> alarmList;

    bool initialized = false;


    Alarm* setAlarm(Time_T start, int32_t repeatAfter, int howMany, Alarm** autoClear) {
        Alarms::begin();
        std::unique_ptr<Alarm> newAlarm(new Alarm(start, repeatAfter, howMany, autoClear));
        newAlarm->autoClear = autoClear;
        Alarm* rawPtr = newAlarm.get();
        alarmList.push_back(std::move(newAlarm));
        return rawPtr;
    }

    void clearAlarm(Alarm*& toBeDeleted) {
        for (auto it = alarmList.begin(); it != alarmList.end(); ++it) {
            if (it->get() == toBeDeleted) {
                alarmList.erase(it);
                break;
            }
        }
    }

    void clearAllAlarms() {
        alarmList.clear();
    }

    void begin() {
        if (Alarms::initialized) {
            return;
        }
        Alarms::initialized = true;
        setInterval([]() {
            for (auto it = alarmList.begin(); it != alarmList.end(); ) {
                Alarm* alarm = it->get();
                if (alarm->isExpired()) {
                    it = alarmList.erase(it);
                } else {
                    ++it;
                }
            }
        }, SECONDS(1));
    }
}
