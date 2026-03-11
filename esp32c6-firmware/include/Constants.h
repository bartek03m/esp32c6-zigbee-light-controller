#pragma once
#include <Arduino.h>

// Hardware pin definitions — I2S microphone
#define I2S_WS 21
#define I2S_SD 23
#define I2S_SCK 22
#define I2S_PORT I2S_NUM_0

// Zigbee pairing button & LED
#define BUTTON_PIN 9
#define LED_PIN 15

// UART to ESP8266
#define UART_TX_PIN 4
#define UART_RX_PIN 5
#define UART_BAUD 9600

// Audio analysis constants
static const int SILENCE_DEBOUNCE = 60;                // How long it must be quiet to consider the clap finished
static const int MAX_CLAP_DURATION = 200;              // Above this, it's background noise
static const int MAX_SEQUENCE_TIME = 5000;
