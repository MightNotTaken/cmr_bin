#pragma once
#include <Arduino.h>
#include <vector>

struct ParamReading {
    uint32_t start;                 // window start (ms)
    float total;                    // sum of accepted samples
    uint32_t count;                 // number of accepted samples
    float minReading;
    float maxReading;
    float currentValue;
    float maxValue;
    float consolidatedReading;      // last window’s average
    bool consolidated;              // true right after a rollover
    uint32_t consolidationDuration; // ms

    std::vector<float> minuteReadings;
    ParamReading(uint32_t consolidationDuration, float minReading, float maxReading);
    float read();
    void   reset(uint32_t now = 0); // reset window and (re)start timer
    float  consolidate();           // compute average for current window
    void   update(float newValue);  // add a sample; rolls window as needed
    int    readLatest();
};
