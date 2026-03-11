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
    
    // Upewnij sie ze ustawiles ssid i haslo w Constants.h!
    initWiFi();   // Initialize WiFi and Web Server
}

// Diagnostic timer
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
        // Sprawdzamy czy WiFi dalej trybi
        if (WiFi.status() == WL_CONNECTED) {
            // Uncomment if you want spam
            // Serial.printf("[DIAG] t=%lus | heap=%u | light=%s\n", 
            //               millis() / 1000, ESP.getFreeHeap(), lightState ? "ON" : "OFF");
        } else {
            Serial.println("[DIAG] !!! WiFi disconnected !!! Reconnecting...");
            WiFi.reconnect();
        }
    }
}