#include "AudioLogic.h"
#include "Globals.h"
#include "Constants.h"
#include "ZigbeeLogic.h"
#include <driver/i2s.h>
#include "zcl/esp_zigbee_zcl_common.h"

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

// Main audio loop: detect amplitude and claps
void processAudio()
{
    int32_t sample = 0;
    size_t bytes_read = 0;

    // Reset sequence if too long
    if (clapCount > 0 && (millis() - firstClapTime > MAX_SEQUENCE_TIME))
    {
        // Serial.println("Timeout. Reset.");
        clapCount = 0;
    }

    // Read audio data
    i2s_read(I2S_PORT, &sample, sizeof(sample), &bytes_read, 0);

    if (bytes_read > 0)
    {
        int32_t raw = sample >> 14;
        int amplitude = abs(raw);

        // Noise gate
        if (amplitude > NOISE_THRESHOLD_START)
        {
            if (!isSoundActive)
            {
                isSoundActive = true;
                soundStartTime = millis();
                maxAmpDuringEvent = amplitude;
            }

            lastLoudMoment = millis();

            if (amplitude > maxAmpDuringEvent)
            {
                maxAmpDuringEvent = amplitude;
            }
        }

        // Silence detected - analyze duration
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
            else if (duration < 5 && maxAmpDuringEvent > MIN_AMPLITUDE_FOR_IMPULSE)
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
                    delay(1500);
                }
            }
        }
    }
}
