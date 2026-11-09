// Zigbee coordinator and On/Off control of the bound device.
#pragma once
#include <Arduino.h>

// Initialize Zigbee stack and pairing
void initZigbee();

// Send Zigbee On/Off command (thread-safe, retried 3x)
void sendZigbeeCommand(uint8_t commandId);
