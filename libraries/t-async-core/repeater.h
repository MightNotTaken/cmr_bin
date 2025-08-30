#ifndef REPEATER_H
#define REPEATER_H

#include <Arduino.h>
#include <functional>
#include <list>
#include <memory>
#include <expire.h>

using RepeaterDuration = unsigned long;

namespace Repeaters {
    struct Repeater;

    Repeater* setRepeater(std::function<void()> callback, RepeaterDuration delay, int count = -1);

    Repeater* setRepeater(std::function<void()> callback, RepeaterDuration delay, int count, Repeater** outHandle);

    void clearRepeater(Repeater*& ref);

    void execute();

    void flush();

    class Repeater: public Expire {
    public:
        std::function<void()> callback; 
        RepeaterDuration delay;         
        unsigned long lastExecution;    
        int count;                      
        Repeater** autoClear;
        Repeater(std::function<void()> cb, RepeaterDuration delay, int count);
        ~Repeater();
        uint32_t getRemainingBeats();
    };
} 

#endif 
