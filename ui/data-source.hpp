#pragma once

#include <event-handler.h>
#include <Arduino.h>

class DataSource: public EventHandler {
public:
  void loop();
};

extern DataSource dataSource;