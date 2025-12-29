#pragma once
#include <Arduino.h>
#include <JSON.h>

#define NODE_ID         1


#define MAIN_CALL_EVENT      "up"
#define MAIN_REPLY_EVENT     String("u") + NODE_ID

#define REG_INIT_EVENT       String("r") + NODE_ID
#define REG_CONF_EVENT       String("c") + NODE_ID

#define SEN_CONF_EVENT       String("sa") + NODE_ID
#define SEN_CONF_REM_EVENT   String("sr") + NODE_ID

namespace Device {
  void begin();
  void loop();
  bool isRegistered();
  void confirmRegistration();
  void setLevel(int level);
  void setTemp(int temperature);
  String getData();
  void addConfiguration(JSON input, bool save = true);
  void removeConfiguration(uint32_t id, bool save = true);
  void saveConfiguration();
  String getConfigPath();
  void loadConfiguration();
  
}