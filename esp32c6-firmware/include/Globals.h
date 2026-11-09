// Shared global state for the ESP32-C6 firmware.
#pragma once
#include <Arduino.h>
#include <Zigbee.h>
#include <Preferences.h>
#include "esp_zigbee_core.h"
#include "ep/ZigbeeSwitch.h"

extern bool lightState;         // Current light state (true = ON)
extern ZigbeeSwitch *mySwitch;  // Zigbee endpoint used to control the Sonoff
extern Preferences preferences; // Flash storage for the paired device address

// Audio globals (clap detection state)
extern int clapCount;                // Claps counted in the current sequence
extern unsigned long firstClapTime;  // Time of the first clap in the sequence
extern unsigned long soundStartTime; // Start of the current loud event
extern unsigned long lastLoudMoment; // Last time the signal was above the threshold
extern bool isSoundActive;           // True while a loud event is in progress
extern int32_t maxAmpDuringEvent;    // Peak amplitude of the current event
extern int32_t micSensitivity;       // Loudness threshold for a clap
