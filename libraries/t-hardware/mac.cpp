#include <mac.h>
#include <WiFi.h>

String mac_response = "";
String mac() {
    if (mac_response.length()) {
        return mac_response;
    }
    char address[20];
    WiFi.macAddress().toCharArray(address, 18);
    mac_response = String(address);
    mac_response.replace(":", "");
    return mac_response;
}