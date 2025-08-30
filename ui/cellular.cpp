#include "cellular.hpp"
#include "display.hpp"
#include "config.hpp"
#include "definitions.h"
#include <JSON.h>
#include <inter-com.hpp>
#include "furnace-ctrl.hpp"
#include <database.h>


namespace Cellular {
    bool ready = false;
    bool genisisDone = false;
    bool connected = false;
    bool internetConnected = false;
    String loopStr;
    String receivedData = "";
    CellularState cellularState = CellularState::CELL_TURN_OFF;
    int networkStrength = 0;
    void delayMS(uint32_t ms) {
        uint32_t start = millis();
        while (millis() - start < ms) {        
            Display::loop();
            interCom.loop();
            FurnaceCtrl::loop();
            AsyncCore::run();
        }
    }

    bool isConnected() {
        return connected; 
    }

    void begin() {
        GSMSerial.begin(115200, SERIAL_8N1, CELLULAR_RX, CELLULAR_TX);
        pinMode(CELLULAR_RST, OUTPUT);
    }

    void initialCommands() {
        Cellular::sendAtCommand("AT", {"OK"});
        Cellular::sendAtCommand("AT+CPIN", {"READY"});
        Cellular::sendAtCommand("AT+CREG=1", {"OK"});
        
        Cellular::sendAtCommand(String("AT+CGDCONT=1,\"IP\",\"") + Configuration::MQTT::apn + "\"", {"OK"});
        Cellular::cellularState = CellularState::CELL_NO_NETWORK;
    }

    
    int _getNetworkStrength() {
        int strength = 0;
        String response = Cellular::sendAtCommand("AT+CSQ", {"+CSQ:"});
        response = filterString(response, "+CSQ:");
        if (response.length()) {
            sscanf(response.c_str(), "+CSQ: %d", &strength);
        }
        if (strength == 99) {
            return 0;
        }
        return strength;
    }

    int getNetworkStrength() {
        return networkStrength;
    }

    void updateNetworkStrength() {
        static uint32_t lastNWUpdate = 0;
        if (millis() - lastNWUpdate > SECONDS(5)) {
            lastNWUpdate = millis();
            networkStrength = _getNetworkStrength();
        }
    }

    void pulse() {
        digitalWrite(CELLULAR_RST, HIGH);
        delayMS(200);
        digitalWrite(CELLULAR_RST, LOW);
        delayMS(1000);
        digitalWrite(CELLULAR_RST, HIGH);
    }

    void turnOn() {
        Serial.println("tuning on");
        pulse();
        Cellular::cellularState = CellularState::CELL_WAIT_TURN_ON;
    }

    String sendAtCommand(String command, std::vector<String> expected, uint32_t timeout) {
        GSMSerial.flush();
        GSMSerial.println(command);
        uint32_t start = millis();
        String response;
        
        while (millis() - start < timeout) {
            if (GSMSerial.available()) {
                while (GSMSerial.available()) {
                    char ch = (char)GSMSerial.read();
                    Serial.write(ch);
                    response += ch;
                    delay(3);
                }
            }
            if (response.indexOf("NORMAL POWER DOWN") > -1) {
                Serial.println("module shut down");
                cellularState = CellularState::CELL_TURN_OFF;
                return "";
            }
            for (auto resp: expected) {
                if (response.indexOf(resp) > -1) {
                    return  response;
                }
            }
            if (response.indexOf("ERROR") > -1) {
                return response;
            }
            
            delayMS(3);
        }
        return response;
    }

    PDPContext activatePDP() {
        String response = sendAtCommand("AT+CGACT?", {"+CGACT:"}, 60000);
        response = filterString(response, "+CGACT:");
        if (response.length()) {
            int context, status;
            sscanf(response.c_str(), "+CGACT: %d,%d", &context, &status);
            if (status == 1) {
                return PDP_CONTEXT_ACTIVATED;
            } else {
                sendAtCommand("AT+CGACT=1,1", {"OK"}, 60000);
            }
        }
        return PDP_CONTEXT_DEACTIVATED;
    }

    
    String filterString(String input, String expected) {
        int index = input.indexOf(expected);
        if (index > -1) {
            return input.substring(index);
        }
        return "";
    }

    bool waitPacketNetwork() {
        String response = sendAtCommand("AT+CGATT?", {"OK"}, 60000);
        response = filterString(response, "+CGATT:");
        if (response.length()) {
            int state;
            sscanf(response.c_str(), "+CGATT: %d", &state);
            if (state == 1) {
                return true;
            }
        }
        sendAtCommand("AT+CGATT=0", {"OK", "ERROR"}, 20000);
        delayMS(1000);
        response = sendAtCommand("AT+CGATT=1", {"OK"}, 20000);
        return false;
    }

    void configureMQTT() {
        String command = String("AT+QMTCFG=\"WILL\",0,1,1,1,\"") + Configuration::MQTT::willTopic + "\",\"" + Configuration::MQTT::willMessage + "\"";
        sendAtCommand(command, {"AT"});
        sendAtCommand("AT+QMTCFG=\"KEEPALIVE\",0,120", {"OK"});
    }

    bool openMQTTConnection() {
        String response = sendAtCommand("AT+QMTOPEN?", {"OK"});
        if (response.indexOf("+QMTOPEN: 0") > -1) {
            return true;
        } else {
            configureMQTT();
            String response = sendAtCommand(String("AT+QMTOPEN=0,\"") + Configuration::MQTT::server + "\"," + Configuration::MQTT::port, {"+QMTOPEN:"}, SECONDS(20));
            response = filterString(response, "+QMTOPEN: ");
            
            if (response.length()) {
                int error;
                int cid;
                sscanf(response.c_str(), "+QMTOPEN: %d,%d", &cid, &error);
                return !error;
            }
        }
        return false;
    }


    void initializeMQTT() {
        while (true) {
            String response = sendAtCommand("AT+QMTCONN?", {"OK"});
            response = filterString(response, "+QMTCONN:");
            if (response.length()) {
                int status;
                sscanf(response.c_str(), "+QMTCONN: %d", &status);
                if (status == 1 || status == 2) {
                    continue;
                } else {
                    break;
                }
            } else {
                break;
            }
        }
    }
    bool connectToMQTT() {
        initializeMQTT();
        String response = sendAtCommand("AT+QMTCONN?", {"OK"});
        if (response.indexOf("+QMTCONN: 0,3") > -1) {
            return true;
        } else {
            String command = "AT+QMTCONN=0,\"CID\",\"USER\",\"PASS\"";
            command.replace("CID", Configuration::MQTT::clientID);
            command.replace("USER", Configuration::MQTT::username);
            command.replace("PASS", Configuration::MQTT::password);
            String response = sendAtCommand(command, {"+QMTCONN:"}, SECONDS(20));
            response = filterString(response, "+QMTCONN:");
            if (response.length()) {
                int id;
                int result;
                int error;
                sscanf(response.c_str(), "+QMTCONN: %d,%d,%d", &id, &result, &error);
                return !error;
            }
        }
        return false;
    }

    bool disconnectFromMQTT() {
        String response = sendAtCommand("AT+QMTDISC=0", {"+QMTDISC:"});
        response = filterString(response, "+QMTDISC:");
        if (response.length()) {
            int error;
            sscanf(response.c_str(), "+QMTDISC: %d,%d", &_, &error);
            return !error;
        }
        return false;
    }

    bool closeMQTTConnection() {
        String response = sendAtCommand("AT+QMTCLOSE=0", {"+QMTCLOSE:"});
        response = filterString(response, "+QMTCLOSE:");
        if (response.length()) {
            int error;
            sscanf(response.c_str(), "+QMTCLOSE: %d,%d", &_, &error);
            return !error;
        }
        return false;
    }

    void listen() {
        sendAtCommand("AT+QMTSUB=0,1,\"" + Configuration::MQTT::clientID + "\",1", {"+QMTSUB:"}, SECONDS(60));
    }

    

    bool emit(String event, String data) {
        static int messageID = 0;
        messageID ++; 
        String response = sendAtCommand(String("AT+QMTPUB=0,") + messageID + ",1,0,\"" + event + "\"", {">"}, SECONDS(5));
        if (response.indexOf(">") > -1) {
            response = sendAtCommand(data + SEPERATOR, {"+QMTPUB:"}, SECONDS(20));
            response = filterString(response, "+QMTPUB:");
            if (response.length()) {
                int messageID, result;
                sscanf(response.c_str(), "+QMTPUB: %d,%d,%d", &_, &messageID, &result);
                if (result == 0) {
                    return true;
                }
            }                
        }
        return false;
    }

    void emitGenesisData() {
        JSON data("[]");
        data.push_back(Configuration::MQTT::clientID);
        data.push_back(Configuration::plantName);
        emit("cmr:connect", data.toString());
    }


    bool waitForNetwork() {
        int strength = getNetworkStrength();
        Cellular::sendAtCommand("AT+CPIN?", {"READY"});
        Cellular::sendAtCommand("AT+CREG=2", {"OK"});
        String response = Cellular::sendAtCommand("AT+CREG?", {"+CREG:"});
        if (response.length()) {
            int state;
            response = response.substring(response.indexOf("+CREG:"));
            sscanf(response.c_str(), "+CREG: %d,%d", &_, &state);
            if (state == NetworkStatus_T::NETWORK_HOME || state == NetworkStatus_T::NETWORK_ROAMING) {
                return true;
            }
        }
        return false;
    }

    bool available() {
        return receivedData.length();
    }

    String read() {
        String response = receivedData;
        receivedData = "";
        return response;
    }

    void listenMQTT() {
        if (GSMSerial.available()) {
            while (GSMSerial.available()) {
                char ch = GSMSerial.read();
                Serial.write(ch);
                loopStr += ch;
                delay(3);                
                if (ch == '\n') {
                    if (loopStr.indexOf("+QMTRECV: ") > -1) {
                        Cellular::genisisDone = true;
                        Cellular::cellularState = CellularState::CELL_FLUSH_LOGS;
                        int index = loopStr.indexOf("{");
                        if (index > -1) {
                            for (int i=loopStr.length() - 1; i>=0; i--) {
                                if (loopStr[i] == '}') {
                                    break;
                                }
                                loopStr[i] = ' ';
                            }
                            String data = loopStr.substring(index);
                            data.trim();
                            receivedData = data;
                            loopStr = "";
                        }

                        return;
                    }
                    if (loopStr.indexOf("+QMTSTAT: ") > -1) {
                        Cellular::connected = false;
                        Cellular::cellularState = CellularState::CELL_WAIT_TURN_ON;
                        return;
                    }
                    loopStr = "";
                    continue;
                }
            }            
        }
    }

    void flushLogs() {
        for (auto& [mac, furnace]: FurnaceCtrl::list) {
            String filename = furnace->getLogFile();
            if (database.hasFile(filename)) {
                File file = SPIFFS.open(filename, FILE_READ);
                if (file) {
                    String payload = "";
                    int index = 0;
                    int count = 0;
                    
                    while (index ++ < file.size()) {
                        count ++;
                        char ch = file.read();
                        payload += ch;
                        if (count == 1000 || index == file.size()) {
                            Serial.println("emitting saved logs");
                            Cellular::emit("cmr:saved-data", mac + "~" + payload);
                            count = 0;
                        }
                    }
                }
                database.removeFile(furnace->getLogFile());
            }
        }
    }
    

    void loop() {
        static uint32_t attempts = 0;
        updateNetworkStrength();
        switch (cellularState) {
            case CellularState::CELL_TURN_OFF: Cellular::turnOn();
                break;
            case CellularState::CELL_WAIT_TURN_ON: {
                String response = sendAtCommand("AT", {"OK"});
                Cellular::internetConnected = false;
                if (response.indexOf("OK") > -1) {
                    // cellularState = CellularState::CELL_HANDLE_AT;
                    cellularState = CellularState::CELL_GENESIS;
                    sendAtCommand("AT+CFUN=1", {"OK"}, 10000);
                }
            } break;
            case CellularState::CELL_HANDLE_AT: {
                if (Serial.available()) {
                    String cmd = Serial.readString();
                    cmd.trim();
                    sendAtCommand(cmd, {"OK"}, 20000);
                }
            } break;
            case CellularState::CELL_GENESIS: Cellular::initialCommands();
                break;
            case CellularState::CELL_NO_NETWORK: if (Cellular::waitForNetwork()) {
                cellularState = CellularState::CELL_NO_PACKET_DATA;
            } break;
            case CellularState::CELL_NO_PACKET_DATA: if (Cellular::waitPacketNetwork()) {
                cellularState = CellularState::CELL_NO_PDP;
            } else {
                attempts ++;
                if (attempts == 100)  {
                    attempts = 0;
                    cellularState = CellularState::CELL_WAIT_TURN_ON;
                }
            } break;
            case CellularState::CELL_NO_PDP: if (Cellular::activatePDP() == PDP_CONTEXT_ACTIVATED) {
                cellularState = CellularState::CELL_OPEN_MQTT;
            } else {
                attempts ++;
                if (attempts == 100)  {
                    attempts = 0;
                    cellularState = CellularState::CELL_WAIT_TURN_ON;
                }
            } break;
            case CellularState::CELL_OPEN_MQTT: {
                if (Cellular::openMQTTConnection()) {
                    cellularState = CellularState::CELL_CONNECT_TO_MQTT;
                } else {
                    Cellular::disconnectFromMQTT();
                    Cellular::closeMQTTConnection();
                    attempts ++;
                    if (attempts == 10)  {
                        attempts = 0;
                        cellularState = CellularState::CELL_WAIT_TURN_ON;
                    }
                }
            } break;
            case CellularState::CELL_CONNECT_TO_MQTT: {
                if (Cellular::connectToMQTT()) {
                    Serial.println("connected to mqtt");
                    Cellular::listen();
                    cellularState = CellularState::CELL_EMIT_GENESIS;
                } else {
                    Cellular::disconnectFromMQTT();
                    cellularState = CellularState::CELL_WAIT_TURN_ON;
                }
            } break;
            case CellularState::CELL_EMIT_GENESIS: {
                genisisDone = false;
                Cellular::emitGenesisData();
                cellularState = CellularState::CELL_LISTEN_MQTT;
            } break;
            case CellularState::CELL_LISTEN_MQTT: {
                Cellular::internetConnected = true;
                Cellular::listenMQTT();
            } break;
            case CellularState::CELL_FLUSH_LOGS: {
                Cellular::flushLogs();
                cellularState = CellularState::CELL_LISTEN_MQTT;
            } break;
        }
    }
}
