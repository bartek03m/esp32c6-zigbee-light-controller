// Line-based UART protocol with the ESP32-C6.
// Sends:    CMD:ON / CMD:OFF
// Receives: STATUS:ON / STATUS:OFF
// The same UART is also used for debug logs; the ESP32-C6 ignores
// every line that does not start with "CMD:".
#include "UartComm.h"
#include "Globals.h"

// Collects incoming characters until a full line arrives
static String uartBuffer = "";

void initUart()
{
    // NodeMCU default Serial: RX = GPIO3, TX = GPIO1
    // Baud rate must match UART_BAUD on the ESP32-C6
    Serial.begin(9600);
    // Print a bit of blank space to separate from bootloader garbage
    Serial.println("\n\n=== ESP8266 UART INIT ===");
}

// Send an ON/OFF command to the ESP32-C6
void sendCommand(bool on)
{
    if (on)
    {
        Serial.println("CMD:ON");
    }
    else
    {
        Serial.println("CMD:OFF");
    }
}

// Read status lines from the ESP32-C6 and update the cached light state
void processUartStatus()
{
    while (Serial.available())
    {
        char c = (char)Serial.read();

        if (c == '\n')
        {
            uartBuffer.trim();

            if (uartBuffer == "STATUS:ON")
            {
                lightState = true;
            }
            else if (uartBuffer == "STATUS:OFF")
            {
                lightState = false;
            }

            uartBuffer = ""; // Clear buffer
        }
        else if (c != '\r') // ignore carriage return
        {
            uartBuffer += c;
        }
    }
}
