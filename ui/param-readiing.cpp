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
    maxValue = 0.0f;
    minuteReadings.clear();
}

float ParamReading::consolidate() {
    std::vector<float> sortedReadings = minuteReadings;
    std::sort(sortedReadings.begin(), sortedReadings.end(), std::greater<float>());
    
    size_t numReadings = std::min(static_cast<size_t>(25), sortedReadings.size());
    if (numReadings > 0) {
        float sum = 0.0f;
        for (size_t i = 0; i < numReadings; ++i) {
            sum += sortedReadings[i];
        }
        consolidatedReading = sum / static_cast<float>(numReadings);
    } else {
        consolidatedReading = 0.0f;
    }
    
    return consolidatedReading;
}

// float ParamReading::consolidate() {

//     if (count == 0) {
//         consolidatedReading = 0.0f;
//     } else {
//         consolidatedReading = total / static_cast<float>(count);
//     }
//     return consolidatedReading;
// }

// void ParamReading::update(float newValue) {
//     currentValue = newValue;
//     if (newValue > maxValue) {
//         maxValue = newValue;
//     }
// }

void ParamReading::update(float newValue) {
    uint32_t now = millis();
    currentValue = newValue;
    if (newValue < minReading) {
        newValue = minReading;
    }

    
    if (newValue > maxReading) {
        newValue = maxReading;
    }
    
    minuteReadings.push_back(newValue);

    total += newValue;
    ++count;
    // consolidate();
}

int ParamReading::readLatest() {
    return static_cast<int>(currentValue);
}

float ParamReading::read() {
    return consolidatedReading;
}
