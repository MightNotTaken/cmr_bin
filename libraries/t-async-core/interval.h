#ifndef INTERVAL_H
#define INTERVAL_H

#include <Arduino.h>
#include <functional>
#include <list>
#include <memory>



using IntervalDuration = unsigned long;

namespace Intervals {
  struct Interval;
  
  Interval* setInterval(std::function<void()> callback, IntervalDuration duration, const String& source = "unknown:interval");
  Interval* setImmediate(std::function<void()> callback, IntervalDuration duration, const String& source = "unknown:immediate");
  
  void clearInterval(Interval*& ref);
  void execute();
  void flush();
  struct Interval {
    std::function<void()> callback; 
    IntervalDuration duration;        
    String source;                    
    unsigned long lastExecution;      
    Interval(std::function<void()> cb, IntervalDuration dur, const String &src);
    ~Interval();
  };
} 

#endif  
