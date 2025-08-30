#include <clock.h>

Clock systemClock;

void Clock::setCurrentTime(Time_T tm) {
    setTime(tm.hour, tm.min, tm.sec, tm.day, tm.month, tm.year % 100);
}

Time_T Clock::getCurrentTime() {
    return Time_T(year(), month(), day(), hour(), minute(), second());
}