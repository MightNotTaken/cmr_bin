#ifndef EVENT_HANDLER_H__
#define EVENT_HANDLER_H__
#include <functional>
#include <Arduino.h>
#include <vector>
#include <memory> // For smart pointers
#include <definitions.h>

#define KEEP_PREVIOUS_LISTENERS         true
#define OVERRIDE_PREVIOUS_LISTENERS     (!KEEP_PREVIOUS_LISTENERS)

typedef std::function<void(String)> EventCallback;
typedef std::function<void()> EventCallbackNoArgument;

struct Event {
  static uint32_t count;
  uint32_t id;
  String name;
  EventCallback callback;
  EventCallbackNoArgument callbackNoArg;

  Event(String name, EventCallback callback);
  Event(String name, EventCallbackNoArgument callback);
  void setCallback(EventCallback cb);
  ~Event();
};

class EventHandler {
  std::vector<std::unique_ptr<Event>> events;
public:
  Event* on(String event, EventCallback callback, bool keepPrevious = true);
  Event* on(String event, EventCallbackNoArgument callback, bool keepPrevious = true);
  bool call(String event, String data = "");
  void unsubscribe(String eventName);
  void unsubscribe(Event* event);
  void prioritize(Event* event);
  ~EventHandler() = default; // No need for manual cleanup
};

#endif
