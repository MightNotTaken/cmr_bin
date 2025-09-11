#include "param-reading.hpp"

ParamReading::ParamReading(uint32_t consolidationDuration,
                           float minReading, float maxReading) {
    this->minReading = minReading;
    this->maxReading = maxReading;
    this->consolidationDuration = consolidationDuration;
    this->consolidatedReading = 0.0f;
    reset(millis());
}

void ParamReading::reset(uint32_t now) {
    if (now == 0) now = millis();
    start = now;
    total = 0.0f;
    count = 0;
}

float ParamReading::consolidate() {
    if (count == 0) {
        consolidatedReading = 0.0f;
    } else {
        consolidatedReading = total / static_cast<float>(count);
    }
    return consolidatedReading;
}

void ParamReading::update(float newValue) {
    uint32_t now = millis();

    // Roll window if time elapsed
    if ((uint32_t)(now - start) >= consolidationDuration) {
        reset(now);
    }

    // Accept only in-range values
    if (newValue >= minReading && newValue <= maxReading) {
        total += newValue;
        ++count;
    }
    consolidate();
}

float ParamReading::read() {
    return consolidatedReading;
}
