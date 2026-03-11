#pragma once
#include <Arduino.h>

// Initialize UART1 for communication with ESP8266
void initUart();

// Check for incoming UART commands (CMD:ON / CMD:OFF) and execute
void processUartCommands();

// Send light status to ESP8266 via UART
void sendStatus(bool state);
