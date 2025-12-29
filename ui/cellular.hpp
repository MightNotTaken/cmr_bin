#pragma once
#include <vector>
#include <utility>
#include <Arduino.h>

#define CELLULAR_RX       18
#define CELLULAR_TX       17
#define CELLULAR_RST      8

#define GSMSerial         Serial1


typedef enum {
    NETWORK_NOT_REGISTERED = 0,
    NETWORK_HOME           = 1,
    NETWORK_SEARCHING      = 2,
    NETWORK_DENIED         = 3,
    NETWORK_UNKNOWN        = 4,
    NETWORK_ROAMING        = 5
} NetworkStatus_T;

enum PDPContext {
    PDP_CONTEXT_DEACTIVATED = 0,
    PDP_CONTEXT_ACTIVATED = 1
};

enum CellularState {
    CELL_TURN_OFF = 0,
    CELL_HANDLE_AT,
    CELL_WAIT_TURN_ON,
    CELL_GENESIS,
    CELL_NO_NETWORK,
    CELL_NO_PACKET_DATA,
    CELL_NO_PDP,
    CELL_OPEN_MQTT,
    CELL_CONNECT_TO_MQTT,
    CELL_EMIT_GENESIS,
    CELL_FLUSH_LOGS,
    CELL_LISTEN_MQTT,
    CELL_FLUSH_SECRET_LOGS
};

namespace Cellular {
    extern bool internetConnected;
    extern bool genisisDone;
    extern CellularState cellularState;
    void begin();
    void turnOn();
    void loop();
    bool connect();
    bool isConnected();
    void delayMS(uint32_t ms);
    bool waitForNetwork();
    PDPContext activatePDP();
    String filterString(String input, String expected);
    void initialCommands();
    void pulse();
    int getNetworkStrength();
    bool available();
    String read();
    String sendAtCommand(String command, std::vector<String> expected, uint32_t timeout=300);
    bool emit(String event, String data);
}