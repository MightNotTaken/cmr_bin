#include <time_t.h>
#include <definitions.h>

Time_T::Time_T() {
    Time_T(0);
}
Time_T::Time_T(uint32_t epochSec) {
    tmElements_t tm;
    breakTime(epochSec, tm);
    hour = tm.Hour;
    min = tm.Minute;
    sec = tm.Second;
    day = tm.Day;
    month = tm.Month;
    year = tm.Year + 1970;
}

Time_T::Time_T(const String& timestamp) {
    sscanf(timestamp.c_str(), "%d/%d/%d %d:%d:%d", &year, &month, &day, &hour, &min, &sec);
    if (year < 100) {
        year += 2000;
    }
}

Time_T::Time_T(int year, uint8_t month, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec):
year(year),
month(month),
day(day),
hour(hour),
min(min),
sec(sec) {}

String Time_T::toString() {
    char stamp[26];
    sprintf(stamp, "%04d/%02d/%02d %02d:%02d:%02d", year, month, day, hour, min, sec);
    return String(stamp);
}

String Time_T::toDateString() {
    char stamp[26];
    sprintf(stamp, "%04d/%02d/%02d", year, month, day);
    return String(stamp);
}

String Time_T::toTimeString() {
    char stamp[26];
    sprintf(stamp, "%02d:%02d:%02d", hour, min, sec);
    return String(stamp);
}




String Time_T::remainingWeeks(uint32_t ms) {
    String prefix;
    if (ms > WEEKS(1)) {
        int weeks = ms / WEEKS(1);
        prefix = String(weeks) + "w";
    }
    return prefix + remainingDays(ms % WEEKS(1));
}
String Time_T::remainingDays(uint32_t ms) {
    String prefix;
    if (ms > DAYS(1)) {
        int days = ms / DAYS(1);
        prefix = String(days) + "d";
    }
    return prefix + remainingHours(ms % DAYS(1));    
}
String Time_T::remainingHours(uint32_t ms) {
    String prefix;
    if (ms > HOURS(1)) {
        int hours = ms / HOURS(1);
        prefix = String(hours) + "h";
    }
    return prefix + remainingMinutes(ms % HOURS(1));   
}
String Time_T::remainingMinutes(uint32_t ms) {
    String prefix;
    if (ms > MINUTES(1)) {
        int minutes = ms / MINUTES(1);
        prefix = String(minutes) + "m";
    }
    return prefix + remainingSeconds(ms % MINUTES(1));
}
String Time_T::remainingSeconds(uint32_t ms) {
    String prefix;
    if (ms > SECONDS(1)) {
        int seconds = ms / SECONDS(1);
        prefix = String(seconds) + "s";
    }
    return prefix;
}

String Time_T::getDuration() {
    return remainingWeeks(SECONDS(this->epoch()));
}

uint32_t Time_T::epoch() {
    tmElements_t tm;        
    tm.Hour = hour;
    tm.Minute = min;
    tm.Second = sec;
    tm.Day = day;
    tm.Month = month;
    tm.Year = year - 1970;
    return (uint32_t)makeTime(tm);
}

Time_T Time_T::operator-(Time_T other) {
    return Time_T(this->epoch() - other.epoch());
}

Time_T Time_T::operator+(Time_T other) {
    return Time_T(this->epoch() + other.epoch());
}

Time_T Time_T::operator/(Time_T other) {
    return Time_T(this->epoch() / other.epoch());
}

Time_T Time_T::operator*(Time_T other) {
    return Time_T(this->epoch() * other.epoch());
}

Time_T Time_T::secondsFromNow(uint32_t seconds) {
    return Time_T(this->epoch() + seconds);
}

Time_T Time_T::minutesFromNow(float minutes) {
    return this->secondsFromNow(minutes * 60);
}

Time_T Time_T::hoursFromNow(float hours) {
    return this->minutesFromNow(hours * 60);
}


bool Time_T::operator==(Time_T other) {
    return this->epoch() == other.epoch();
}

bool Time_T::operator!=(Time_T other) {
    return this->epoch() != other.epoch();
}

bool Time_T::operator<(Time_T other) {
    return this->epoch() < other.epoch();
}

bool Time_T::operator<=(Time_T other) {
    return this->epoch() <= other.epoch();
}

bool Time_T::operator>(Time_T other) {
    return this->epoch() > other.epoch();
}

bool Time_T::operator>=(Time_T other) {
    return this->epoch() >= other.epoch();
}