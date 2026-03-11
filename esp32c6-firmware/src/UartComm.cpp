#include "UartComm.h"
#include "Globals.h"
#include "Constants.h"
#include "ZigbeeLogic.h"
#include "zcl/esp_zigbee_zcl_common.h"

// Buffer for incoming UART data
static String uartBuffer = "";

void initUart()
{
    Serial1.begin(UART_BAUD, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);
    Serial.println("[UART] Initialized on TX=" + String(UART_TX_PIN) + " RX=" + String(UART_RX_PIN));
}

void processUartCommands()
{
    while (Serial1.available())
    {
        char c = Serial1.read();
        if (c == '\n')
        {
            uartBuffer.trim();

            if (uartBuffer == "CMD:ON")
            {
                Serial.println("[UART] Received CMD:ON");
                lightState = true;
                sendZigbeeCommand(ESP_ZB_ZCL_CMD_ON_OFF_ON_ID);
                sendStatus(lightState);
            }
            else if (uartBuffer == "CMD:OFF")
            {
                Serial.println("[UART] Received CMD:OFF");
                lightState = false;
                sendZigbeeCommand(ESP_ZB_ZCL_CMD_ON_OFF_OFF_ID);
                sendStatus(lightState);
            }
            else
            {
                Serial.println("[UART] Unknown command: " + uartBuffer);
            }

            uartBuffer = "";
        }
        else
        {
            uartBuffer += c;
        }
    }
}

void sendStatus(bool state)
{
    String msg = state ? "STATUS:ON" : "STATUS:OFF";
    Serial1.println(msg);
    Serial.println("[UART] Sent: " + msg);
}
