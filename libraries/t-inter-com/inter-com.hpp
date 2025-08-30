#pragma once

#include <functional>
#include <event-handler.h>
#include <definitions.h>
#include <async-core.h>

#define BRIDGE_LOG_ENABLED            true
#define BRIDGE_LOG_DISABLED           false

class InterCom: public EventHandler {
    String data;
    bool log;
    bool listening;
    std::function<void()> beforeTransmitCB;
    std::function<void()> afterTransmitCB;
public:
    InterCom();
    void begin(bool logStatus = BRIDGE_LOG_ENABLED);
    void setBaudRate(uint32_t baud);
    void flush();
    void flushSerial();
    void emit(const String& event, const  String& message="");
    void emit(const String& event, const uint32_t& message);
    void loop();
    void enableLog();
    void disableLog();
    uint32_t dataLength();
    void beforeTransmit(std::function<void()> cb);
    void afterTransmit(std::function<void()> cb);
};

namespace InterComBridges {
    void loop();
};
extern InterCom interCom;