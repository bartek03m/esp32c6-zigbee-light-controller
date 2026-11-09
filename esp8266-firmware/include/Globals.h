// Shared global objects for the ESP8266 firmware.
#pragma once
#include <Arduino.h>
#include <ESP8266WebServer.h>

extern ESP8266WebServer server; // HTTP server on port 80
extern bool lightState;         // Last known light state, reported by the ESP32-C6
