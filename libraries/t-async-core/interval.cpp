#include "interval.h"
#include <console.h>

namespace Intervals {

    static std::list<std::unique_ptr<Interval>> intervalList;
    Interval::Interval(std::function<void()> cb, IntervalDuration dur, const String &src)
        : callback(std::move(cb)), duration(dur), source(src), lastExecution(millis()) {}
    
    Interval::~Interval() {
        // console.log("interval erased");
    }
    
    Interval *setImmediate(std::function<void()> callback, IntervalDuration duration, const String &source) {
        callback();
        return setInterval(callback, duration, source);
    }

    Interval *setInterval(std::function<void()> callback, IntervalDuration duration, const String &source) {
        std::unique_ptr<Interval> interval(new Interval(std::move(callback), duration, source));
        Interval *rawPtr = interval.get();
        intervalList.push_back(std::move(interval));
        return rawPtr;
    }
    static void removeInterval(Interval *toBeDeleted) {
        for (auto it = intervalList.begin(); it != intervalList.end(); ++it) {
            if (it->get() == toBeDeleted) {
                intervalList.erase(it);
                break;
            }
        }
    }
    void clearInterval(Interval *&ref) {
        if (ref) {
            removeInterval(ref);
            ref = nullptr;
        }
    }
    void execute() {
        unsigned long now = millis();

        for (auto &interval : intervalList) {

            if (now - interval->lastExecution >= interval->duration) {
                interval->lastExecution = now;
                if (interval->callback) {
                    interval->callback();
                }
            }
        }
    }
    void flush() {
        intervalList.clear();
    }

}
