#pragma once
#include <Arduino.h>

// Initialize Zigbee stack and pairing
void initZigbee();

// Update target address from bound devices
void updateTargetAddress();

// Send Zigbee On/Off command
void sendZigbeeCommand(uint8_t commandId);
