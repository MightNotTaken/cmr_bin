#include "inter-com.hpp"

InterCom interCom;
#define serial (&Serial2)

InterCom::InterCom() {
    this->log = true;
    this->listening = true;
    this->beforeTransmitCB = nullptr;
    this->afterTransmitCB = nullptr;
}

void InterCom::enableLog() {
    this->log = true;
}

void InterCom::disableLog() {
    this->log = false;
}

void InterCom::begin(bool logStatus) {
    if (logStatus == BRIDGE_LOG_ENABLED) {
        this->enableLog();
    } else {
        this->disableLog();
    }
}

void InterCom::setBaudRate(uint32_t baud) {
    serial->end();
    serial->begin(baud);
}

void InterCom::flush() {
    data = "";
}

void InterCom::flushSerial() {
    while (serial->available()) {
        serial->read();
    }
}

void InterCom::emit(const String& event, const  String& message) {
    invoke(beforeTransmitCB);
    serial->write('~');
    serial->print(event);
    serial->write(SEPERATOR);
    serial->print(message);
    serial->write('|');
    serial->flush();
    invoke(afterTransmitCB);
}

void InterCom::beforeTransmit(std::function<void()> cb) {
    beforeTransmitCB = cb;
}

void InterCom::afterTransmit(std::function<void()> cb) {
    afterTransmitCB = cb;
}


void InterCom::emit(const String& event, const uint32_t& message) {
    emit(event, String(message));
}

uint32_t InterCom::dataLength() {
    return this->data.length();
}

void InterCom::loop() {
    while (serial->available()) {
        char ch = serial->read();
        delay(3);
        Serial.write(ch);
        if (ch == '~') {
            this->flush();
            this->listening = true;
            continue;
        }
        if (ch == '|') {
            this->listening = false;
            int index = this->data.indexOf(SEPERATOR);
            String event;
            String dataToEmit;
            if (index > -1) {
                event = this->data.substring(0, index);
                dataToEmit = this->data.substring(index + 1);
            } else {
                dataToEmit = "";
                event = this->data;
            }
            call(event, dataToEmit);
            this->flush();
            continue;
        }
        if (!listening && ch == '\n') {
            call("raw-data", data);
            data = "";
        } else {
            data += ch;
        }
    }
}

namespace InterComBridges {
    void loop() {
        interCom.loop();
    }
}