#pragma once
#include <Arduino.h>

// Initialize WiFi, mDNS and Web Server
void initWiFi();

// Web Server Handlers
void handleRoot();
void handleLightOn();
void handleLightOff();
void handleSensitivity();
void handleStatus();
void handleNotFound();
