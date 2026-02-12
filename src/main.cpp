#include <Arduino.h>
#include "Globals.h"
#include "Constants.h"
#include "WebLogic.h"
#include "ZigbeeLogic.h"
#include "AudioLogic.h"

// System setup
void setup()
{
    // Serial.begin(115200);
    initZigbee(); // Initialize Zigbee stack
    initWiFi();   // Initialize WiFi and Web Server
    initAudio();  // Initialize Audio I2S
    startAudioTask(); // Start audio on a dedicated FreeRTOS task (core 0)
}

// Main processing loop
void loop()
{
    server.handleClient(); // Handle HTTP requests
    updateTargetAddress(); // Check for Zigbee device updates
}