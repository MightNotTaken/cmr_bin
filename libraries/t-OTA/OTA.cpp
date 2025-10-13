#include "OTA.h"
#include <async-core-web-server.h>
#include <Update.h>
#include <JSON.h>
#include <async-core.h>
#include <timeout.h>
#include <console.h>

using namespace Timeouts;
OTA ota;

void OTA::begin() {
    coreWebServer->on("register-call", [this](String) {
        console.log("registering ota routes");
        this->configureRoutes();
    });
}

void OTA::onProgress(std::function<void()> progressCallback) {
    this->progressCallback = progressCallback;
}

void OTA::configureRoutes() {
    coreWebServer->getActualServer()->on("/update", HTTP_POST, [](AsyncWebServerRequest *request) {
        JSON rsp;
        rsp["status"] = Update.hasError() ? "error" : "success";
        AsyncWebServerResponse *response = request->beginResponse(200, "application/json", rsp.toString().c_str());
        response->addHeader("Connection", "close");
        coreWebServer->handleCors(response);
        request->send(response);
        setTimeout([]() {
            ESP.restart();
        },2000);
    }, [this](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
        if (!index) {
            Serial.printf("Update: %s\n", filename.c_str());
            if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
                Update.printError(Serial);
            }
        }
        if (Update.write(data, len) != len) {
            Update.printError(Serial);
        }
        invoke(progressCallback);
        if (final) {
            if (Update.end(true)) {
                Serial.printf("Update Success: %u\nRebooting...\n", index + len);
                JSON rsp;
                rsp["status"] = Update.hasError() ? "error" : "success";

                AsyncWebServerResponse *response = request->beginResponse(200, "application/json", rsp.toString().c_str());
                response->addHeader("Connection", "close");
                coreWebServer->handleCors(response);
                request->send(response);
                delay(1000);
                ESP.restart();
            } else {
                Update.printError(Serial);
            }
        }
    });
}