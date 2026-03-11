#include <Arduino.h>
#include "Globals.h"
#include "Constants.h"
#include "ZigbeeLogic.h"
#include "AudioLogic.h"
#include "UartComm.h"

// System setup
void setup()
{
    Serial.begin(115200);
    delay(500);
    Serial.println("\n=== ESP32-C6: Zigbee + Clap ===");
    initZigbee();     // Initialize Zigbee stack
    initUart();       // Initialize UART to ESP8266
    initAudio();      // Initialize Audio I2S
    startAudioTask(); // Start audio on a dedicated FreeRTOS task
}

// Main processing loop
void loop()
{
    processUartCommands(); // Handle commands from ESP8266
}