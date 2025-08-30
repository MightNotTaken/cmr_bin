#ifndef TIMEOUT_H
#define TIMEOUT_H

#include <Arduino.h>
#include <functional>
#include <list>
#include <memory>

using TimeoutDuration = unsigned long;

namespace Timeouts {
    struct Timeout;
    Timeout* setTimeout(std::function<void()> callback, TimeoutDuration delay);
    Timeout* setTimeout(std::function<void()> callback, TimeoutDuration delay, Timeout** outHandle);

    void clearTimeout(Timeout*& ref, bool log = false);
    void execute();
    void flush();
    struct Timeout {
        uint32_t id;
        uint32_t getID() const { return id; }
        static uint32_t count;
        std::function<void()> callback; 
        TimeoutDuration delay;          
        unsigned long startTime;        
        Timeout** autoClear;
        Timeout(std::function<void()> cb, TimeoutDuration del);
        ~Timeout();
    };
} 

#endif 
