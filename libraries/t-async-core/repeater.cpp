#include "repeater.h"
#include <console.h>

namespace Repeaters {

    static std::list<std::unique_ptr<Repeater>> repeaterList;

    Repeater::Repeater(std::function<void()> cb, RepeaterDuration delay, int count)
        : callback(std::move(cb)), delay(delay), lastExecution(millis()), count(count), autoClear(nullptr) {
    }

    uint32_t Repeater::getRemainingBeats() {
        return count - 1;
    } 
    Repeater::~Repeater() {
        if (autoClear) {
            *autoClear = nullptr;
        }
        // console.log("repeater erased");
    }

    Repeater *setRepeater(std::function<void()> callback, RepeaterDuration delay, int count) {
        std::unique_ptr<Repeater> repeater(new Repeater(std::move(callback), delay, count));
        Repeater *rawPtr = repeater.get();
        repeaterList.push_back(std::move(repeater));
        return rawPtr;
    }

    Repeater *setRepeater(std::function<void()> callback, RepeaterDuration delay, int count, Repeater **outHandle) {
        std::unique_ptr<Repeater> repeater(new Repeater(std::move(callback), delay, count));
        repeater->autoClear = outHandle;
        Repeater *rawPtr = repeater.get();
        repeaterList.push_back(std::move(repeater));
        return rawPtr;
    }

    static void removeRepeater(Repeater *toBeDeleted) {
        for (auto it = repeaterList.begin(); it != repeaterList.end(); ++it) {
            if (it->get() == toBeDeleted) {
                repeaterList.erase(it);
                break;
            }
        }
    }

    void clearRepeater(Repeater *&ref) {
        if (ref) {
            removeRepeater(ref);
            ref = nullptr;
        }
    }


    void execute() {
        unsigned long now = millis();
        for (auto it = repeaterList.begin(); it != repeaterList.end();) {
            Repeater *rep = it->get();
            if (now - rep->lastExecution >= rep->delay) {
                rep->lastExecution = now;
                if (rep->callback) {
                    rep->callback();
                }

                if (rep->count != -1) {
                    rep->count--;

                    if (rep->count <= 0) {
                        rep->expire();
                        it = repeaterList.erase(it);
                        continue;
                    }
                }
            }
            ++it;
        }
    }

    void flush() {
        for (auto it = repeaterList.begin(); it != repeaterList.end();) {
            Repeater *rep = it->get();
            if (rep->count != -1 && rep->count <= 0) {
                if (rep->autoClear) {
                    *rep->autoClear = nullptr;
                }
                it = repeaterList.erase(it);
            }
            else {
                ++it;
            }
        }
    }

}
