#ifndef CONFIG_H__
#define CONFIG_H__
#include <Arduino.h>
namespace Configuration {
    extern String plantName;
    namespace InterCom {
        extern byte tx;
        extern byte rx;
        extern byte de_re;
        void begin();
    };
    namespace MQTT {
        extern String apn;
        extern String server;
        extern int port;
        extern String username;
        extern String password;
        extern String clientID;
        extern String willTopic;
        extern String willMessage;
        void begin();
    };
    void begin();
};
#endif