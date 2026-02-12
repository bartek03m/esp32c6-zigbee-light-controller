#include "AudioLogic.h"
#include "Globals.h"
#include "Constants.h"
#include "ZigbeeLogic.h"
#include <driver/i2s.h>
#include "zcl/esp_zigbee_zcl_common.h"

// Audio sample buffer
static const int AUDIO_BATCH_SIZE = 256;
static int32_t sampleBuffer[AUDIO_BATCH_SIZE];

// Cooldown after toggling
static unsigned long lastToggleTime = 0;
static const unsigned long TOGGLE_COOLDOWN = 1500;

// FreeRTOS task handle
static TaskHandle_t audioTaskHandle = NULL;

// Initialize I2S microphone
void initAudio()
{
    /* I2S Configuration */
    const i2s_config_t i2s_config = {
        .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = 44100,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 256,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0};

    const i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_SCK,
        .ws_io_num = I2S_WS,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = I2S_SD};

    i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_PORT, &pin_config);
    i2s_start(I2S_PORT);
}

// Dedicated FreeRTOS task for audio processing
void audioTask(void *parameter)
{
    while (true)
    {
        // Still in cooldown, just drain the buffer
        if (lastToggleTime > 0 && (millis() - lastToggleTime < TOGGLE_COOLDOWN))
        {
            // Drain the I2S buffer to prevent overflow during cooldown
            size_t bytes_read = 0;
            i2s_read(I2S_PORT, sampleBuffer, sizeof(sampleBuffer), &bytes_read, portMAX_DELAY);
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }
        lastToggleTime = 0;

        // Reset clap sequence if too long
        if (clapCount > 0 && (millis() - firstClapTime > MAX_SEQUENCE_TIME))
        {
            clapCount = 0;
        }

        // Read audio samples
        size_t bytes_read = 0;
        i2s_read(I2S_PORT, sampleBuffer, sizeof(sampleBuffer), &bytes_read, portMAX_DELAY);

        if (bytes_read > 0)
        {
            int samplesRead = bytes_read / sizeof(int32_t);

            // Find peak amplitude
            int maxAmplitude = 0;
            for (int i = 0; i < samplesRead; i++)
            {
                int32_t raw = sampleBuffer[i] >> 14;
                int amplitude = abs(raw);
                if (amplitude > maxAmplitude)
                {
                    maxAmplitude = amplitude;
                }
            }

            // Check if loud enough
            if (maxAmplitude > micSensitivity)
            {
                if (!isSoundActive)
                {
                    isSoundActive = true;
                    soundStartTime = millis();
                    maxAmpDuringEvent = maxAmplitude;
                }

                lastLoudMoment = millis();

                if (maxAmplitude > maxAmpDuringEvent)
                {
                    maxAmpDuringEvent = maxAmplitude;
                }
            }

            // Silence after sound event
            if (isSoundActive && (millis() - lastLoudMoment > SILENCE_DEBOUNCE))
            {
                unsigned long duration = lastLoudMoment - soundStartTime;
                isSoundActive = false;

                bool validClap = false;

                // Check if it looks like a clap
                if (duration >= 5 && duration <= MAX_CLAP_DURATION)
                {
                    validClap = true;
                }
                else if (duration < 5 && maxAmpDuringEvent > micSensitivity)
                {
                    validClap = true;
                }

                // Valid clap detected
                if (validClap)
                {
                    clapCount++;
                    if (clapCount == 1)
                        firstClapTime = millis();

                    // If 3 claps, toggle light
                    if (clapCount == 3)
                    {
                        lightState = !lightState;

                        if (lightState)
                        {
                            sendZigbeeCommand(ESP_ZB_ZCL_CMD_ON_OFF_ON_ID);
                        }
                        else
                        {
                            sendZigbeeCommand(ESP_ZB_ZCL_CMD_ON_OFF_OFF_ID);
                        }
                        clapCount = 0;
                        // Start cooldown
                        lastToggleTime = millis();
                    }
                }
            }
        }

        // Small yield to let other tasks breathe (1ms)
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

// Start audio task so it doesn't get blocked by WiFi/Zigbee
void startAudioTask()
{
    xTaskCreate(
        audioTask,        // Task function
        "AudioTask",      // Name
        4096,             // Stack size (bytes)
        NULL,             // Parameter
        2,                // Priority (higher than loop task = 1, so audio gets served first)
        &audioTaskHandle  // Task handle
    );
}
