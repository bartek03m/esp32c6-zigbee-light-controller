#pragma once
#include <Arduino.h>

// Initialize UART communication with ESP32-C6
void initUart();

// Send ON/OFF command to ESP32-C6
void sendCommand(bool on);

// Process incoming status updates from ESP32-C6
void processUartStatus();
