#pragma once
#include <Arduino.h>

// Initialize I2S microphone driver
void initAudio();

// Start dedicated FreeRTOS task for audio processing
void startAudioTask();