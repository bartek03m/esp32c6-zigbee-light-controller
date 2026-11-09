// Hardware pins and tuning constants for the ESP32-C6 firmware.
#pragma once
#include <Arduino.h>

// Hardware pin definitions — I2S microphone (DFRobot SEN0327)
#define I2S_WS 21  // Word select (LR clock)
#define I2S_SD 23  // Serial data
#define I2S_SCK 22 // Bit clock
#define I2S_PORT I2S_NUM_0

// Zigbee pairing button & LED
#define BUTTON_PIN 9 // BOOT button on the Beetle ESP32-C6
#define LED_PIN 15   // On-board LED, lit while the pairing window is open

// UART to ESP8266
#define UART_TX_PIN 4
#define UART_RX_PIN 5
#define UART_BAUD 9600 // Must match Serial.begin() on the ESP8266

// Audio analysis constants (all times in ms)
static const int SILENCE_DEBOUNCE = 60;   // How long it must be quiet to consider the clap finished
static const int MAX_CLAP_DURATION = 200; // Above this, it's background noise
static const int MAX_SEQUENCE_TIME = 5000; // All 3 claps must happen within this window
