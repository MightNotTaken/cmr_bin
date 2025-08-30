#include "alert-system.hpp"

AlertSystem alertSystem;

void AlertSystem::begin() {
    pinMode(SOUND_PIN, OUTPUT);
    pinMode(GREEN_PIN, OUTPUT);
    pinMode(RED_PIN, OUTPUT);
    pinMode(YELLOW_PIN, OUTPUT);
    shutdown();
}

void AlertSystem::shutdown() {
    activate(ALL_ALERTS_OFF);
}

void AlertSystem::activate(AlertState_T state) {
    digitalWrite(SOUND_PIN, state & SOUND_ON);
    digitalWrite(RED_PIN, state & RED_ON);
    digitalWrite(GREEN_PIN, state & GREEN_ON);
    digitalWrite(YELLOW_PIN, state & YELLOW_ON);
    this->state = state;
    show();
}

void AlertSystem::show() {
    Serial.printf("SOUND %s\n", state & SOUND_ON ? "ON" : "OFF");
    Serial.printf("RED %s\n", state & RED_ON ? "ON" : "OFF");
    Serial.printf("GREEN %s\n", state & GREEN_ON ? "ON" : "OFF");
    Serial.printf("YELLOW %s\n\n", state & YELLOW_ON ? "ON" : "OFF");
}