#ifndef TIME_CUSTOM_H__
#define TIME_CUSTOM_H__
#include <Arduino.h>
#include <TimeLib.h>
class Time_T {
public:
    int hour;
    int min;
    int sec;
    int day;
    int month;
    int year;
    Time_T();
    Time_T(uint32_t epochSec);
    Time_T(const String& timestamp);
    Time_T(int year, uint8_t month, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec);
    String toString();
    String toDateString();
    String toTimeString();
    String getDuration();
    uint32_t epoch();
    Time_T operator-(Time_T other);
    Time_T operator+(Time_T other);
    Time_T operator/(Time_T other);
    Time_T operator*(Time_T factor);
    Time_T secondsFromNow(uint32_t seconds);
    Time_T minutesFromNow(float minutes);
    Time_T hoursFromNow(float hours);
    
    bool operator==(Time_T other);
    bool operator!=(Time_T other);
    bool operator<(Time_T other);
    bool operator<=(Time_T other);
    bool operator>(Time_T other);
    bool operator>=(Time_T other);
private:
    String remainingWeeks(uint32_t ms);
    String remainingDays(uint32_t ms);
    String remainingHours(uint32_t ms);
    String remainingMinutes(uint32_t ms);
    String remainingSeconds(uint32_t ms);
};

#endif
