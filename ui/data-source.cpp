#include "data-source.hpp"
#include <JSON.h>
#include "cellular.hpp"

DataSource dataSource;

void DataSource::loop() {
  if (Cellular::available()) {
    JSON recData(Cellular::read());
    String event = recData["event"].toString();
    String data = recData["data"].toString();
    if (event.length()) {
      call(event, data);
    }
  }
}