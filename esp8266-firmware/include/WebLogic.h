// WiFi, mDNS and HTTP server for the web UI.
#pragma once
#include <Arduino.h>

// Initialize WiFi, mDNS and Web Server
void initWiFi();

// Web Server Handlers: /, /on, /off, /status and 404
void handleRoot();
void handleLightOn();
void handleLightOff();
void handleStatus();
void handleNotFound();
