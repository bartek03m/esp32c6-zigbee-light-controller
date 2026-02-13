#include <Arduino.h>
#include "Globals.h"
#include "Constants.h"
#include "WebLogic.h"
#include "ZigbeeLogic.h"
#include "AudioLogic.h"
#include <WiFi.h>

// System setup
void setup()
{
    Serial.begin(115200);
    delay(500);
    Serial.println("\n=== DIAGNOSTYKA START ===");
    initZigbee(); // Initialize Zigbee stack
    initWiFi();   // Initialize WiFi and Web Server
    initAudio();  // Initialize Audio I2S
    startAudioTask(); // Start audio on a dedicated FreeRTOS task (core 0)
}

// Diagnostic timer
static unsigned long lastDiag = 0;

// Main processing loop
void loop()
{
    server.handleClient(); // Handle HTTP requests

    // === DIAGNOSTYKA: co 10 sekund loguj status systemu ===
    if (millis() - lastDiag > 10000)
    {
        lastDiag = millis();
        Serial.printf("[DIAG] t=%lus | heap=%u | WiFi=%s | bound=%s | light=%s\n",
                      millis() / 1000,
                      esp_get_free_heap_size(),
                      WiFi.status() == WL_CONNECTED ? "OK" : "LOST",
                      mySwitch->bound() ? "YES" : "NO",
                      lightState ? "ON" : "OFF");
    }
}