#include "UartComm.h"
#include "Globals.h"

static String uartBuffer = "";

void initUart()
{
    // NodeMCU Default Serial: RX = GPIO3, TX = GPIO1
    Serial.begin(9600);
    // Print a bit of blank space to separate from bootloader garbage
    Serial.println("\n\n=== ESP8266 UART INIT ===");
}

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
