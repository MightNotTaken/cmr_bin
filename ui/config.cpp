#include "config.hpp"
#include "mac.h"
namespace Configuration {
    String plantName = "Unnamed";
    namespace InterCom {
        byte rx = 16;
        byte tx = 15;
        void begin() {
        }
    };
    
    namespace MQTT {
        String apn = "airtelgprs.com";

        String server = "45.248.62.119";
        int port = 1883;
        String username = "otis";
        String password = "Cmr!@!century@12";
        
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