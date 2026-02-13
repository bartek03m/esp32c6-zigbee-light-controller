#pragma once
#include <Arduino.h>

// Initialize Zigbee stack and pairing
void initZigbee();

// Send Zigbee On/Off command
void sendZigbeeCommand(uint8_t commandId);
