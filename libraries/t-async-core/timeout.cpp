#include "timeout.h"
#include <console.h>

namespace Timeouts {

  uint32_t Timeout::count = 0;
  static std::list<std::unique_ptr<Timeout>> timeoutList;

  
  Timeout::Timeout(std::function<void()> cb, TimeoutDuration del)
    : callback(std::move(cb)), delay(del), startTime(millis()), autoClear(nullptr)
  {
    count ++;
    id = count;
  }

  Timeout::~Timeout() {
    // console.log("timeout erased");
    if (autoClear) {
      *autoClear = nullptr;
    }
  }
  
  static void removeTimeout(Timeout* toBeDeleted) {
    for (auto it = timeoutList.begin(); it != timeoutList.end(); ++it) {
      if (it->get() == toBeDeleted) {
        timeoutList.erase(it);
        break;
      }
    }
  }

  
  Timeout* setTimeout(std::function<void()> callback, TimeoutDuration delay) {
    std::unique_ptr<Timeout> timeout(new Timeout(std::move(callback), delay));
    Timeout* rawPtr = timeout.get();
    timeoutList.push_back(std::move(timeout));
    return rawPtr;
  }

  
  Timeout* setTimeout(std::function<void()> callback, TimeoutDuration delay, Timeout** outHandle) {
    std::unique_ptr<Timeout> timeout(new Timeout(std::move(callback), delay));
    timeout->autoClear = outHandle;
    Timeout* rawPtr = timeout.get();
    timeoutList.push_back(std::move(timeout));
    return rawPtr;
  }

  
  void clearTimeout(Timeout*& ref, bool log) {
    if (log) {
      console.log("clearing timeout");
    }

    if (ref) {
      if (log) {
        console.log("timeout cleared", ref->getID());
      }
      removeTimeout(ref);
      ref = nullptr;
    }
  }

  
  
  void execute() {
    unsigned long now = millis();
    for (auto it = timeoutList.begin(); it != timeoutList.end(); ) {
      Timeout* t = it->get();
      if (now - t->startTime >= t->delay) {
        if (t->callback) {
          t->callback();
        }
        it = timeoutList.erase(it);
      } else {
        ++it;
      }
    }
  }

  
  
  void flush() {
    unsigned long now = millis();
    for (auto it = timeoutList.begin(); it != timeoutList.end(); ) {
      Timeout* t = it->get();
      if (now - t->startTime >= t->delay) {
        
        if (t->autoClear) {
          *t->autoClear = nullptr;
        }
        it = timeoutList.erase(it);
      } else {
        ++it;
      }
    }
  }

} 
