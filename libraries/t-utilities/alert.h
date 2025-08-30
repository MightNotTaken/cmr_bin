#ifndef ALERT_H__
#define ALERT_H__

#include <Arduino.h>
#include <event-handler.h>

enum Alert_Direction_T
{
    ALERT_UPWARD = 0,
    ALERT_DOWNWARD = 1
};

template <class T>
class Alert : public EventHandler
{
private:
    T threshold;
    T currentValue;
    T recovery;
    T limit;
    bool triggered;
    Alert_Direction_T direction;

public:
    Alert(T threshold, T recovery, T limit, Alert_Direction_T direction)
        : threshold(threshold),
          recovery(recovery),
          limit(limit),
          triggered(true),
          direction(direction)
    {
    }
    void update(T newValue)
    {
        currentValue = newValue;

        if (direction == ALERT_UPWARD)
        {
            if (currentValue > threshold && currentValue <= limit)
            {
                if (!triggered)
                {
                    triggered = true;
                    EventHandler::call("trigger");
                }
            }
            else if (triggered && currentValue < (threshold - recovery))
            {
                triggered = false;
                EventHandler::call("recover");
            }
        }
        else // ALERT_DOWNWARD
        {
            if (currentValue < threshold && currentValue >= limit)
            {
                if (!triggered)
                {
                    triggered = true;
                    EventHandler::call("trigger");
                }
            }
            else if (triggered && currentValue > (threshold + recovery))
            {
                triggered = false;
                EventHandler::call("recover");
            }
        }
    }
};

#endif
