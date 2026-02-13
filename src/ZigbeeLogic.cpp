#include "ZigbeeLogic.h"
#include "Globals.h"
#include "Constants.h"
#include <Arduino.h>
#include <Zigbee.h>
#include "esp_zigbee_core.h"
#include "zcl/esp_zigbee_zcl_common.h"
#include "esp_coexist.h"
#include <list>

// Setup Zigbee, pairing button, and preferences
void initZigbee()
{
    // Preferences init
    preferences.begin("zb_config", false);

    delay(2000);

    pinMode(BUTTON_PIN, INPUT_PULLUP); // Boot button for pairing mode
    pinMode(LED_PIN, OUTPUT);          // LED to let user know about pairing mode
    bool pairing = false;
    if (digitalRead(BUTTON_PIN) == LOW) // if u press the button during boot, enable pairing mode
    {
        pairing = true;
    }

    /* Zigbee */
    // Set channel Zigbee to 25
    uint32_t channel_mask = (1 << 25);
    Zigbee.setPrimaryChannelMask(channel_mask);
    delay(2000);

    // Create ZigbeeSwitch endpoint — this properly configures ON/OFF clusters
    // so that binding with Sonoff (or any ON/OFF light) works automatically
    mySwitch = new ZigbeeSwitch(1);

    // Add to main Zigbee object
    Zigbee.addEndpoint(mySwitch);

    // Start as a coordinator
    if (!Zigbee.begin(ZIGBEE_COORDINATOR, pairing))
    {
        Serial.println("[ZB] !!! Zigbee start FAILED !!!");
        while (1)
            delay(100);
    }
    Serial.println("[ZB] Zigbee started OK");

    // coexistence WiFi + Zigbee
    esp_coex_preference_set(ESP_COEX_PREFER_BALANCE);
    esp_err_t coex_err = esp_coex_wifi_i154_enable();
    Serial.printf("[ZB] Coex enable result: %d (0=OK)\n", coex_err);

    // If pairing mode active, open network for joining
    if (pairing)
    {
        Serial.println("[ZB] PAIRING MODE - network open for 180s");
        Zigbee.openNetwork(180); // Open network for 180 seconds (more time!)
        digitalWrite(LED_PIN, HIGH);
        delay(180000); // wait for 180 seconds to allow full pairing + binding
        digitalWrite(LED_PIN, LOW);
        Serial.println("[ZB] Pairing window closed");

        // Log binding result
        if (mySwitch->bound())
        {
            Serial.println("[ZB] Device BOUND successfully!");
            std::list<zb_device_params_t *> devices = mySwitch->getBoundDevices();
            for (auto dev : devices)
            {
                Serial.printf("[ZB]   -> addr=0x%04X, endpoint=%d\n", dev->short_addr, dev->endpoint);
            }
        }
        else
        {
            Serial.println("[ZB] WARNING: No device bound after pairing window!");
        }
    }
    delay(2000);
    Serial.println("[ZB] initZigbee complete");
}

// Send ON/OFF command to Zigbee device using ZigbeeSwitch API
void sendZigbeeCommand(uint8_t commandId)
{
    if (!mySwitch->bound())
    {
        Serial.printf("[ZB-CMD] NOT BOUND - cannot send cmd=%d | t=%lus\n", commandId, millis() / 1000);
        return;
    }

    // Get the bound device's address and endpoint
    std::list<zb_device_params_t *> devices = mySwitch->getBoundDevices();
    if (devices.empty())
    {
        Serial.println("[ZB-CMD] No bound devices found!");
        return;
    }
    zb_device_params_t *dev = devices.front();

    Serial.printf("[ZB-CMD] Sending cmd=%d to 0x%04X ep=%d | heap=%u | t=%lus\n",
                  commandId, dev->short_addr, dev->endpoint, esp_get_free_heap_size(), millis() / 1000);

    // Use explicit unicast with device address (not binding table mode)
    if (commandId == ESP_ZB_ZCL_CMD_ON_OFF_ON_ID)
    {
        mySwitch->lightOn(dev->endpoint, dev->short_addr);
    }
    else if (commandId == ESP_ZB_ZCL_CMD_ON_OFF_OFF_ID)
    {
        mySwitch->lightOff(dev->endpoint, dev->short_addr);
    }
    else
    {
        mySwitch->lightToggle(dev->endpoint, dev->short_addr);
    }

    Serial.println("[ZB-CMD] Command sent OK (unicast)");
}
