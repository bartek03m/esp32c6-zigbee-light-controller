#pragma once
#include <Arduino.h>
#include <driver/i2s.h>

// Hardware pin definitions
#define I2S_WS 21
#define I2S_SD 23
#define I2S_SCK 22
#define I2S_PORT I2S_NUM_0
#define BUTTON_PIN 9
#define LED_PIN 15

// Audio analysis constants
static const int32_t NOISE_THRESHOLD_START = 4000;     // Threshold to start analysis
static const int SILENCE_DEBOUNCE = 60;                // How long it must be quiet to consider the clap finished
static const int MAX_CLAP_DURATION = 200;              // Above this, it's background noise
static const int32_t MIN_AMPLITUDE_FOR_IMPULSE = 4000; // If time=0ms, amplitude must be at least this high
static const int MAX_SEQUENCE_TIME = 5000;

// WiFi credentials
static const char *ssid = "";
static const char *password = "";
