#ifndef DEFINITION_H__
#define DEFINITION_H__
#include <Arduino.h>
#include <memory>
#define NULL_REFERENCE   0

#define SEPERATOR      ''
#define invoke(func, ...)  do {\
  if (func) {\
    func(__VA_ARGS__);\
  }\
} while (0)
#if __cplusplus < 201402L
namespace std {
  template<typename T, typename... Args>
  std::unique_ptr<T> make_unique(Args&& ... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
  }
}
#endif
extern int _;

#define SECONDS(x)     ((uint64_t)(x * 1000U))
#define MINUTES(x)     (uint64_t)(x * SECONDS(60l))
#define HOURS(x)       (uint64_t)(x * MINUTES(60l))
#define DAYS(x)        (uint64_t)(x * HOURS(24l))
#define WEEKS(x)       (uint64_t)(x * DAYS(7l))

#define _SECONDS_(x)   (uint64_t)(x)
#define _MINUTES_(x)   (uint64_t)(x * _SECONDS_(60l))
#define _HOURS_(x)     (uint64_t)(x * _MINUTES_(60l))
#define _DAYS_(x)      (uint64_t)(x * _HOURS_(24l))
#define _WEEKS_(x)     (uint64_t)(x * _DAYS_(7l))

String formatDays(int days, uint8_t hours = 0);
String formatHours(uint16_t hours, uint8_t minutes = 0, uint8_t seconds = 0);
String formatMinutes(uint16_t minutes, uint8_t seconds = 0);
String formatSeconds(uint32_t seconds, uint16_t ms = 0);
String formatMillis(uint32_t ms);

#endif