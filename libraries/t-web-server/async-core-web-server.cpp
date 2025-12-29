#include "async-core-web-server.h"
#include <console.h>

CoreWebServer* coreWebServer = new CoreWebServer(80);

CoreWebServer::CoreWebServer(int port) {
  webserver = new AsyncWebServer(port);
}

CoreWebServer::~CoreWebServer() {
  delete webserver;
}

AsyncWebServer* CoreWebServer::getActualServer() {
  return webserver;
}

void CoreWebServer::get(String route, ServerPredicate callback) {
  webserver->on(route.c_str(), HTTP_OPTIONS, [this](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse(204);
    this->handleCors(response);
    request->send(response);
  });
  webserver->on(route.c_str(), HTTP_GET, [callback, this, route](Request *request) {
    this->_query = JSON("{}");
    for (size_t i = 0; i < request->params(); i++) {
      String key = request->getParam(i)->name();
      String value = request->getParam(i)->value();
      this->_query[key] = value;
    }
    callback(request, [this, &request](int code, String data) {
      AsyncWebServerResponse *response = request->beginResponse(code, "application/json", data.c_str());
      this->handleCors(response);
      request->send(response);
    });
  });
}

void CoreWebServer::post(String route, ServerPredicate callback, std::function<void(AsyncWebServerRequest*, String, size_t, uint8_t*, size_t, bool)> filehandler) {
  webserver->on(route.c_str(), HTTP_OPTIONS, [this](AsyncWebServerRequest *request) {
    // Handle preflight OPTIONS request
    AsyncWebServerResponse *response = request->beginResponse(204);
    this->handleCors(response);
    request->send(response);
  });
  webserver->on(
    route.c_str(), HTTP_POST, [route, this, callback](Request *request) {
      this->_query = JSON("{}");
      for (size_t i = 0; i < request->params(); i++) {
        String key = request->getParam(i)->name();
        String value = request->getParam(i)->value();
        this->_query[key] = value;
      }
      callback(request, [this, &request](int code, String data) {
        AsyncWebServerResponse *response = request->beginResponse(code, "application/json", data.c_str());
        this->handleCors(response);
        request->send(response);
      });
    },
    filehandler, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      if (index == 0) {
        if (data[0] == '{' || data[0] == '[') {
          while (len >= 0) {
            if (data[len] == '}' || data[len] == ']') {
              break;
            }
            data[len] = '\0';
            len --;
          }
        }
        this->_body.load((char*)data);
      }
    });    
}

void CoreWebServer::stop() {
  webserver->end();
  MDNS.end();
}

void CoreWebServer::begin(String domain) {
  this->call("register-call");
  console.log("register call");
  webserver->begin();
  _query = JSON::nullJSON;
  if (!MDNS.begin(domain.c_str())) {
    console.log("unable to grab mdns");
  } else {
    console.log("grabbed mdns");
  }
}

JSON CoreWebServer::body() {
  JSON bd = this->_body;
  this->_body = JSON::nullJSON;
  return bd;
}

JSON CoreWebServer::query() {
  JSON qu = this->_query;
  this->_query = JSON::nullJSON;
  return qu;
}

void CoreWebServer::handleCors(AsyncWebServerResponse* response) {
    response->addHeader("Access-Control-Allow-Origin", "*");
    response->addHeader("Access-Control-Allow-Methods", "POST, OPTIONS, GET, PUT, DELETE");
    response->addHeader("Access-Control-Allow-Headers", "Content-Type");      
}