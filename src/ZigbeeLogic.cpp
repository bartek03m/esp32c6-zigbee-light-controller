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
    // Read stored target address, default to broadcast if not set
    targetZigbeeAddress = preferences.getUShort("addr", 0xFFFF);

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

    // Create endpoint
    myEP = new ZigbeeEP(1);

    // Add to main Zigbee object
    Zigbee.addEndpoint(myEP);

    // Start as a coordinator
    if (!Zigbee.begin(ZIGBEE_COORDINATOR, pairing))
    {
        // Serial.println("Zigbee start error");
        while (1)
            delay(100);
    }
    // Serial.println("Zigbee started");
    // coexistence WiFi + Zigbee
    esp_coex_preference_set(ESP_COEX_PREFER_BALANCE);
    esp_err_t coex_err = esp_coex_wifi_i154_enable();

    // If pairing mode active, open network for joining
    if (pairing)
    {
        Zigbee.openNetwork(30); // Open network for 30 seconds
        digitalWrite(LED_PIN, HIGH);
        delay(30000); // wait for 30 seconds to get stable connection (WiFI can interrupt Zigbee connection at this time)
        digitalWrite(LED_PIN, LOW);
    }
    delay(2000);
}

// Check for bound devices and update target address
void updateTargetAddress()
{
    // We check if we have an endpoint created
    if (myEP != NULL)
    {
        // Get the list of bound devices from the endpoint
        std::list<zb_device_params_t *> devices = myEP->getBoundDevices();

        if (!devices.empty())
        {
            // Get the first device from the list
            zb_device_params_t *device = devices.front();

            // If the device has a valid short address, update our target address
            if (device->short_addr != 0 && device->short_addr != 0xFFFF && device->short_addr != targetZigbeeAddress)
            {
                targetZigbeeAddress = device->short_addr;

                // Store the new target address in preferences
                preferences.putUShort("addr", targetZigbeeAddress);
            }
        }
    }
}

// Send ON/OFF command to Zigbee device
void sendZigbeeCommand(uint8_t commandId)
{
    // Declare a structure variable to hold the On/Off command request parameters
    esp_zb_zcl_on_off_cmd_t cmd_req;

    // Zero out the structure memory
    memset(&cmd_req, 0, sizeof(cmd_req));

    // Set the source endpoint to 1
    cmd_req.zcl_basic_cmd.src_endpoint = 1;

    // Set the addressing mode depending on address type
    if (targetZigbeeAddress == 0xFFFF)
    {
        // Broadcast if we don't know the Sonoff address yet
        cmd_req.address_mode = ESP_ZB_APS_ADDR_MODE_16_ENDP_PRESENT;
        // Serial.println("Sending BROADCAST (0xFFFF)");
    }
    else
    {
        // Unicast if we have a specific Sonoff address
        cmd_req.address_mode = ESP_ZB_APS_ADDR_MODE_16_ENDP_PRESENT;
        // Serial.printf("Sending UNICAST to 0x%04x\n", targetZigbeeAddress);
    }

    // Set the specific command ID to 'Toggle' (switch state)
    cmd_req.on_off_cmd_id = commandId;

    // Set destination address to 0xFFFF (Broadcast to all routers and end devices - we have only one so it works fine)
    cmd_req.zcl_basic_cmd.dst_addr_u.addr_short = targetZigbeeAddress;

    // Set the destination endpoint to 1 (Sonoff devices typically listen on endpoint 1)
    cmd_req.zcl_basic_cmd.dst_endpoint = 1;

    // Execute the Zigbee On/Off command request using the configured structure
    esp_zb_lock_acquire(portMAX_DELAY);
    esp_zb_zcl_on_off_cmd_req(&cmd_req);
    esp_zb_lock_release();
}
