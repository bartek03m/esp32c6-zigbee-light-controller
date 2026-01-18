#pragma once
#include <Arduino.h>
#include <WebServer.h>
#include <Zigbee.h>
#include <Preferences.h>
#include "esp_zigbee_core.h"

extern bool lightState;
extern ZigbeeEP *myEP;
extern uint16_t targetZigbeeAddress;
extern Preferences preferences;
extern WebServer server;

// Audio globals
extern int clapCount;
extern unsigned long firstClapTime;
extern unsigned long soundStartTime;
extern unsigned long lastLoudMoment;
extern bool isSoundActive;
extern int32_t maxAmpDuringEvent;
