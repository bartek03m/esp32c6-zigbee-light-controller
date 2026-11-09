// ESP8266 entry point: WiFi, web server and UART bridge to the ESP32-C6.
#include <Arduino.h>
#include <ESP8266mDNS.h>
#include "Globals.h"
#include "WebLogic.h"
#include "UartComm.h"

// System setup
void setup()
{
    initUart();   // Initialize UART to ESP32-C6
    delay(500);
    Serial.println("\n=== ESP8266: Web Server + WiFi ===");
    
    // Make sure WiFi credentials are set in include/Secrets.h
    initWiFi();   // Initialize WiFi and Web Server
}

// Timestamp of the last diagnostic check
static unsigned long lastDiag = 0;

// Main processing loop
void loop()
{
    // Handle HTTP requests from clients
    server.handleClient(); 
    
    // Handle incoming UART from ESP32-C6 (checking if light state changed from clap)
    processUartStatus();
    
    // MDNS update
    MDNS.update();

    // Diagnostics every 5s
    if (millis() - lastDiag > 5000)
    {
        lastDiag = millis();
        // Check that WiFi is still connected, reconnect if not
        if (WiFi.status() == WL_CONNECTED)
        {
            // Uncomment for periodic status logs (verbose)
            // Serial.printf("[DIAG] t=%lus | heap=%u | light=%s\n",
            //               millis() / 1000, ESP.getFreeHeap(), lightState ? "ON" : "OFF");
        }
        else
        {
            Serial.println("[DIAG] !!! WiFi disconnected !!! Reconnecting...");
            WiFi.reconnect();
        }
    }
}