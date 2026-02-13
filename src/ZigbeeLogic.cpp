#include "ZigbeeLogic.h"
#include "Globals.h"
#include "Constants.h"
#include <Arduino.h>
#include <Zigbee.h>
#include "esp_zigbee_core.h"
#include "zcl/esp_zigbee_zcl_common.h"
#include "esp_coexist.h"
#include <list>

// IEEE (MAC) address of paired device never changes unlike short address
static esp_zb_ieee_addr_t savedIeeeAddr = {0};
static uint8_t savedEndpoint = 1;
static bool hasSavedDevice = false;

// Thread safety: mutex + throttle between commands
static SemaphoreHandle_t zbCmdMutex = NULL;
static unsigned long lastCmdTime = 0;
static const unsigned long MIN_CMD_INTERVAL = 300; // min 300ms between commands

// Helper: print IEEE address
static void printIeeeAddr(const char *prefix, const esp_zb_ieee_addr_t addr)
{
    Serial.printf("%s %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\n", prefix,
                  addr[7], addr[6], addr[5], addr[4],
                  addr[3], addr[2], addr[1], addr[0]);
}

// Helper: check if IEEE address is all zeros
static bool isZeroAddr(const esp_zb_ieee_addr_t addr)
{
    for (int i = 0; i < 8; i++)
    {
        if (addr[i] != 0)
            return false;
    }
    return true;
}

// Setup Zigbee, pairing button, and preferences
void initZigbee()
{
    // Preferences init
    preferences.begin("zb_config", false);

    // Create mutex for thread-safe command sending
    zbCmdMutex = xSemaphoreCreateMutex();

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
        Zigbee.openNetwork(180); // Open network for 180 seconds
        digitalWrite(LED_PIN, HIGH);
        delay(180000); // wait for pairing + binding
        digitalWrite(LED_PIN, LOW);
        Serial.println("[ZB] Pairing window closed");

        // Check binding result and save IEEE address to Preferences
        if (mySwitch->bound())
        {
            std::list<zb_device_params_t *> devices = mySwitch->getBoundDevices();
            if (!devices.empty())
            {
                zb_device_params_t *dev = devices.front();
                memcpy(savedIeeeAddr, dev->ieee_addr, sizeof(esp_zb_ieee_addr_t));
                savedEndpoint = dev->endpoint;
                hasSavedDevice = true;

                // Persist IEEE address to flash (8 bytes)
                preferences.putBytes("dev_ieee", savedIeeeAddr, sizeof(esp_zb_ieee_addr_t));
                preferences.putUChar("dev_ep", savedEndpoint);

                Serial.printf("[ZB] Device BOUND! short=0x%04X ep=%d\n", dev->short_addr, dev->endpoint);
                printIeeeAddr("[ZB] IEEE addr saved:", savedIeeeAddr);
            }
        }
        else
        {
            Serial.println("[ZB] WARNING: No device bound after pairing window!");
        }
    }
    else
    {
        // Normal boot - load saved IEEE address from Preferences
        size_t len = preferences.getBytes("dev_ieee", savedIeeeAddr, sizeof(esp_zb_ieee_addr_t));
        savedEndpoint = preferences.getUChar("dev_ep", 1);

        if (len == sizeof(esp_zb_ieee_addr_t) && !isZeroAddr(savedIeeeAddr))
        {
            hasSavedDevice = true;
            printIeeeAddr("[ZB] Loaded saved IEEE:", savedIeeeAddr);
        }
        else
        {
            Serial.println("[ZB] No saved device — hold BOOT on next start to pair!");
        }
    }

    delay(2000);
    Serial.println("[ZB] initZigbee complete");
}

// Send ON/OFF command using permanent IEEE address
// Thread-safe: mutex prevents audio task and web handler from colliding
void sendZigbeeCommand(uint8_t commandId)
{
    if (!hasSavedDevice)
    {
        Serial.printf("[ZB-CMD] No paired device — cannot send cmd=%d\n", commandId);
        return;
    }

    // Take mutex — wait max 1 second, then give up
    if (xSemaphoreTake(zbCmdMutex, pdMS_TO_TICKS(1000)) != pdTRUE)
    {
        Serial.println("[ZB-CMD] Mutex timeout — skipping command");
        return;
    }

    // Throttle: wait if last command was sent less than 300ms ago
    unsigned long now = millis();
    unsigned long elapsed = now - lastCmdTime;
    if (elapsed < MIN_CMD_INTERVAL)
    {
        delay(MIN_CMD_INTERVAL - elapsed);
    }

    Serial.printf("[ZB-CMD] Sending cmd=%d via IEEE ep=%d | heap=%u | t=%lus\n",
                  commandId, savedEndpoint, esp_get_free_heap_size(), millis() / 1000);

    // Use IEEE (MAC) address
    if (commandId == ESP_ZB_ZCL_CMD_ON_OFF_ON_ID)
    {
        mySwitch->lightOn(savedEndpoint, savedIeeeAddr);
    }
    else if (commandId == ESP_ZB_ZCL_CMD_ON_OFF_OFF_ID)
    {
        mySwitch->lightOff(savedEndpoint, savedIeeeAddr);
    }
    else
    {
        mySwitch->lightToggle(savedEndpoint, savedIeeeAddr);
    }

    lastCmdTime = millis();
    Serial.println("[ZB-CMD] Command sent OK (IEEE unicast)");

    // Release mutex 
    xSemaphoreGive(zbCmdMutex);
}
