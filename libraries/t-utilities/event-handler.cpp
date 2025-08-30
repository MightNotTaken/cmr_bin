
#include "event-handler.h"
#include <console.h>

uint32_t Event::count = 0;

Event::Event(String name, EventCallback callback)
  : name(name), callback(callback), callbackNoArg(nullptr) {
    Event::count++;
    this->id = Event::count;
}

Event::Event(String name, EventCallbackNoArgument callback)
  : name(name), callback(nullptr), callbackNoArg(callback) {
    Event::count++;
    this->id = Event::count;
}

Event::~Event() {
}

void Event::setCallback(EventCallback cb) {
  this->callback = cb;
}

void EventHandler::unsubscribe(String eventName) {
  events.erase(std::remove_if(events.begin(), events.end(), [&](const std::unique_ptr<Event>& e) {
    return e->name == eventName;
  }), events.end());
  events.shrink_to_fit();
}

void EventHandler::unsubscribe(Event* event) {
  if (!event) return;
  console.log("removing event");
  events.erase(std::remove_if(events.begin(), events.end(), [&](const std::unique_ptr<Event>& e) {
    return e->id == event->id;
  }), events.end());
  console.log("event removed");
}

Event* EventHandler::on(String event, EventCallback callback, bool keepPrevious) {
  if (!keepPrevious) {
    this->unsubscribe(event);
  }
  events.push_back(std::make_unique<Event>(event, callback));
  return events.back().get();
}

Event* EventHandler::on(String event, EventCallbackNoArgument callback, bool keepPrevious) {
  if (!keepPrevious) {
    this->unsubscribe(event);
  }
  events.push_back(std::make_unique<Event>(event, callback));
  return events.back().get();
}

void EventHandler::prioritize(Event* event) {
  auto it = std::find_if(events.begin(), events.end(), [&](const std::unique_ptr<Event>& e) {
    return e.get() == event;
  });
  if (it != events.end()) {
    std::unique_ptr<Event> temp = std::move(*it);
    events.erase(it);
    events.insert(events.begin(), std::move(temp));
  }
}

bool EventHandler::call(String event, String data) {
  bool touched = false;
  for (const auto& e : events) {
    if (e->name == event) {
      invoke(e->callback, data);
      invoke(e->callbackNoArg);
      touched = true;
    }
  }
  return touched;
}
