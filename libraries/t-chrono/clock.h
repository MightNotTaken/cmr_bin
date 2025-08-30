#ifndef CLOCK_H__
#define CLOCK_H__
#include <time_t.h>
#include <TimeLib.h>

class Clock {
public:
    void setCurrentTime(Time_T);
    Time_T getCurrentTime();
};
extern Clock systemClock;

#endif
