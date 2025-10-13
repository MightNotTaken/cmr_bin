#ifndef OTA_H__
#define OTA_H__
#include <event-handler.h>
#include <functional>
class OTA: public EventHandler {
    std::function<void()> progressCallback;
public:
    void configureRoutes();
    void begin();
    void onProgress(std::function<void()> progressCallback);
};

extern OTA ota;
#endif