#ifndef ASYNC_WEB_SERVER_H__
#define ASYNC_WEB_SERVER_H__

#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#endif
#include <functional>
#include <utility>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <JSON.h>
#include <event-handler.h>

#define Request AsyncWebServerRequest

typedef std::function<void(int, String)> CodeResponsePair;

typedef std::function<
    void(Request*, CodeResponsePair)
> ServerPredicate;


class CoreWebServer : public EventHandler {
    int port;
    JSON _body;
    JSON _query;
    AsyncWebServer* webserver; 
public:
    CoreWebServer(int);
    ~CoreWebServer();
    void get(String, ServerPredicate);
    void post(String, ServerPredicate, std::function<void(AsyncWebServerRequest*, String, size_t, uint8_t*, size_t, bool)> = NULL);
    void begin(String);
    void stop();
    JSON body();
    JSON query();
    void handleCors(AsyncWebServerResponse* response);
    AsyncWebServer* getActualServer();
private:
};
extern CoreWebServer* coreWebServer;
#endif