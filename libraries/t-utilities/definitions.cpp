#include "definitions.h"

int _;


String formatDays(int days, uint8_t hours) {
  return String(days) + "d " + (hours ? String(hours) + "h" : ""); 
}

String formatHours(uint16_t hours, uint8_t minutes, uint8_t seconds) {
  if (hours < 24) {
    return String(hours) + ":" + minutes + ":" + seconds;
  }
  return formatDays(hours / 24.0, hours % 24);
}

String formatMinutes(uint16_t minutes, uint8_t seconds) {
  if (minutes < 60) {
    return String(minutes) + "m " + (seconds ? String(seconds) + "s" : "");
  }
  return formatHours(minutes / 60.0, minutes % 60, seconds);
}

String formatSeconds(uint32_t seconds, uint16_t ms) {
  if (seconds < 60) {
    return String(seconds) + "s " + (ms ? String(ms) + "ms" : "");
  }
  return formatMinutes(seconds / 60.0, seconds % 60);
}

String formatMillis(uint32_t ms) {
  if (ms < 1000) {
    return String(ms) + "ms";
  }
  return formatSeconds(ms / 1000.0, ms % 1000);
}
