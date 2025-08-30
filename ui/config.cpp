#include "config.hpp"
#include "mac.h"
namespace Configuration {
    String plantName = "Ahmedabad";
    namespace InterCom {
        byte tx = 15;
        byte rx = 16;
        void begin() {
        }
    };
    
    namespace MQTT {
        String apn = "airtelgprs.com";

        String server = "178.16.137.20";
        int port = 1883;
        String username = "tahirbhaijaan";
        String password = "AlMustafa@786";
        
        String clientID = mac();
        String willTopic = "cmr:shutdown";
        String willMessage;
        void begin() {
            willMessage = mac();
            clientID = mac();
        }
    };

    void begin() {
        MQTT::begin();
        InterCom::begin();
    }
};