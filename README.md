# 👏 Zigbee Light Controller

A custom-built smart home controller running on the DFRobot Beetle ESP32-C6. It listens for claps or commands from a local website to control a Sonoff Zigbee Switch. It runs completely locally using Zigbee and WiFi.

## 🔦 The "Why"
I built this because my girlfriend, Jula, is too lazy to get out of bed to turn off the lights (well, I'm too lazy too). Now she can just clap three times and boom, darkness. If clapping is too much effort, she can also control it via a Web Interface from her phone.

## 🛠️ Hardware
- MCU: DFRobot Beetle ESP32-C6 V1.1 (Ultra-compact RISC-V board with native Zigbee).
- Mic: DFRobot SEN0327.
- Target: Sonoff ZBMINI-L2 (No Neutral Wire Zigbee Relay) - hidden behind the wall switch!

## ⚡ Hardware Setup & Wiring (Important!)
Before flashing, ensure your hardware is set up correctly:
1. Microphone Connection: Connect the SEN0327 to the Beetle ESP32-C6 - refer to the provided schematic/image.
2. Sonoff Installation:
    * WARNING: The Sonoff ZBMINI-L2 involves working with High Voltage (110V/230V).
    * It must be installed behind your wall switch or near the light bulb.
    * Ensure power is OFF at the breaker before installing the Sonoff!

## ⚙️ How it works
1. Audio Analysis: The code continuously reads audio levels from the microphone. It filters background noise and detects loud impulses (claps).
2. Logic: If 3 valid claps are detected within a specific time window (e.g., 5 seconds), it triggers the action.
3. Zigbee: The ESP32 acts as a Zigbee Coordinator. It sends explicit On or Off commands to the bound Sonoff.
4. Web Interface & WiFi: The board connects to WiFi and hosts a cyberpunk-themed web interface (`http://light.local`). You can use it to control the light manually if you don't feel like clapping.

## 🚀 Installation
1. Clone the repo.
2. Open in VS Code with PlatformIO.
3. Build and Upload to the Beetle ESP32-C6.
4. Open the Serial Monitor to see debug logs and the IP address (Note: You may need to uncomment specific Serial print lines in the code to see debug info).

## 🖱️ Usage
- Pairing: Hold the BOOT button on the Beetle while powering on to open the Zigbee network for pairing. Put the Sonoff ZBMINI-L2 into pairing mode (usually by holding its button for 5s or flipping the light switch on and off several times).
- Control (Sound): Clap 3 times to switch the light.
- Control (WiFi): Connect to `http://light.local` and click the On/Off button.

---
*Built by Bartek.*
