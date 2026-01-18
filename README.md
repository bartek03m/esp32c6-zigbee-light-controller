# 👏 Zigbee Clap Controller

A custom-built smart home controller running on the ESP32-C6. It listens for claps and toggles a Zigbee light bulb. Simple, effective, and runs completely locally.

## 🔦 The "Why"
Let's be real - I built this because my girlfriend Jula is too lazy to get out of bed to turn off the lights (well I'm to lazy too). Now she can just clap three times and boom, darkness. Also she can control it from the web interface. I'm just a good guy solving real-world problems.

## 🛠️ Hardware
- MCU: ESP32-C6 DevKitC-1 (RISC-V with native Zigbee!)
- Mic: DFRobot SEN0327 (High-quality I2S MEMS Microphone)
- Target: Sonoff ZBMINI-L2 (No Neutral Wire Zigbee Relay) - hidden behind the wall switch! 

## ⚙️ How it works
1. Audio Analysis: Code reads audio samples via I2S. It looks for loud "impluses" (claps) and ignores background noise.
2. Logic: If 3 valid claps are detected within 5 seconds, it triggers the action.
3. Zigbee: The ESP32 acts as a Zigbee Coordinator. It sends a standard `Toggle` command to the bound light bulb.
4. Web Interface: There's also a cool cyberpunk-themed web interface hosted on the ESP for manual control if you lose your hands.

## 🚀 Installation
1. Clone the repo.
2. Open in VS Code with PlatformIO.
3. Build and Upload.
4. Open the Serial Monitor to see debug logs.

## 🖱️ Usage
- Pairing: Hold the BOOT button while powering on to enter pairing mode (LED will light up).
- Control: Clap 3 times to toggle the light.
- Web: Connect to `http://light.local` (or the IP logged in Serial).

---
*Built with ❤️ (and laziness) by Bartek.*
