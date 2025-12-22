#include <Arduino.h>
#include <Network.h> 
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <Zigbee.h> 
#include "esp_zigbee_core.h" 
#include "zcl/esp_zigbee_zcl_common.h"
#include <driver/i2s.h>

// PINS
#define I2S_WS 21
#define I2S_SD 23
#define I2S_SCK 22
#define I2S_PORT I2S_NUM_0


/* Global const and variables */

// ZigBee
ZigbeeSwitch *mySwitch = NULL;

// Mic
const int32_t NOISE_THRESHOLD_START = 12000; // Threshold to start analysis
const int SILENCE_DEBOUNCE = 60; // How long it must be quiet to consider the clap finished
const int MAX_CLAP_DURATION = 150; // Above this, it's background noise
const int32_t MIN_AMPLITUDE_FOR_IMPULSE = 12000; // If time=0ms, amplitude must be at least this high
const int MAX_SEQUENCE_TIME = 2500; 
int clapCount = 0;
unsigned long firstClapTime = 0;
unsigned long soundStartTime = 0;
unsigned long lastLoudMoment = 0;
bool isSoundActive = false;
int32_t maxAmpDuringEvent = 0;

// Local Website
const char* ssid = "Project_hotspot";
const char* password = "pwd12345";
WebServer server(80);

// Website by Google Gemini saved in flash memory
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pl">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>GŁÓWNY PANEL STEROWANIA</title>
    <link href="https://fonts.googleapis.com/css2?family=Share+Tech+Mono&display=swap" rel="stylesheet">
    <style>
        :root { --primary: #00f3ff; --primary-dim: #00f3ff44; --alert: #ff0055; --alert-dim: #ff005544; --bg: #050505; --panel: #0a0a0a; --grid: #1a1a1a; --font-main: 'Share Tech Mono', monospace; }
        * { box-sizing: border-box; user-select: none; }
        body { background-color: var(--bg); color: var(--primary); font-family: var(--font-main); margin: 0; height: 100vh; overflow: hidden; display: flex; justify-content: center; align-items: center; background-image: linear-gradient(var(--grid) 1px, transparent 1px), linear-gradient(90deg, var(--grid) 1px, transparent 1px); background-size: 30px 30px; }
        body::after { content: " "; display: block; position: absolute; top: 0; left: 0; bottom: 0; right: 0; background: linear-gradient(rgba(18, 16, 16, 0) 50%, rgba(0, 0, 0, 0.25) 50%), linear-gradient(90deg, rgba(255, 0, 0, 0.06), rgba(0, 255, 0, 0.02), rgba(0, 0, 255, 0.06)); z-index: 2; background-size: 100% 2px, 3px 100%; pointer-events: none; }
        .interface-container { width: 90%; max-width: 600px; background: rgba(10, 10, 10, 0.9); border: 1px solid var(--primary); box-shadow: 0 0 20px var(--primary-dim), inset 0 0 30px rgba(0,0,0,0.8); position: relative; padding: 20px; border-radius: 5px; backdrop-filter: blur(5px); z-index: 1; }
        .corner { position: absolute; width: 20px; height: 20px; border: 2px solid var(--primary); transition: all 0.3s ease; }
        .tl { top: -2px; left: -2px; border-right: none; border-bottom: none; } .tr { top: -2px; right: -2px; border-left: none; border-bottom: none; } .bl { bottom: -2px; left: -2px; border-right: none; border-top: none; } .br { bottom: -2px; right: -2px; border-left: none; border-top: none; }
        header { border-bottom: 1px solid var(--primary-dim); padding-bottom: 15px; margin-bottom: 20px; display: flex; justify-content: space-between; align-items: center; text-shadow: 0 0 5px var(--primary); }
        h1 { margin: 0; font-size: 24px; letter-spacing: 2px; text-transform: uppercase; }
        .sys-data { font-size: 12px; color: rgba(255,255,255,0.7); text-align: right; }
        .main-display { display: flex; flex-direction: column; align-items: center; margin-bottom: 30px; position: relative; }
        .reactor-core { width: 150px; height: 150px; border-radius: 50%; border: 5px solid #333; position: relative; display: flex; justify-content: center; align-items: center; margin: 20px 0; background: #000; box-shadow: inset 0 0 20px #000; transition: all 0.5s cubic-bezier(0.175, 0.885, 0.32, 1.275); }
        .reactor-inner { width: 80%; height: 80%; border-radius: 50%; background: radial-gradient(circle, #333 0%, #000 70%); opacity: 0.5; transition: all 0.5s ease; position: relative; }
        .reactor-core.active { border-color: var(--primary); box-shadow: 0 0 30px var(--primary), inset 0 0 20px var(--primary); }
        .reactor-core.active .reactor-inner { background: radial-gradient(circle, #fff 0%, var(--primary) 60%, transparent 100%); opacity: 1; animation: pulse 2s infinite; }
        .reactor-ring { position: absolute; width: 100%; height: 100%; border-radius: 50%; border: 2px dashed #333; animation: spin 10s linear infinite; }
        .reactor-core.active .reactor-ring { border-color: var(--primary); animation: spin 4s linear infinite; }
        .status-text { font-size: 1.5rem; margin-top: 15px; letter-spacing: 3px; font-weight: bold; }
        .status-on { color: var(--primary); text-shadow: 0 0 10px var(--primary); display: none; } .status-off { color: var(--alert); text-shadow: 0 0 10px var(--alert); }
        .controls { display: grid; grid-template-columns: 1fr 1fr; gap: 20px; width: 100%; }
        .btn { background: transparent; border: 2px solid; padding: 20px; font-family: var(--font-main); font-size: 20px; font-weight: bold; cursor: pointer; text-transform: uppercase; position: relative; overflow: hidden; transition: all 0.3s; clip-path: polygon(10% 0, 100% 0, 100% 90%, 90% 100%, 0 100%, 0 10%); }
        .btn-on { color: var(--primary); border-color: var(--primary); box-shadow: inset 0 0 10px var(--primary-dim); } .btn-off { color: var(--alert); border-color: var(--alert); box-shadow: inset 0 0 10px var(--alert-dim); }
        .btn::before { content: ''; position: absolute; top: 0; left: -100%; width: 100%; height: 100%; background: linear-gradient(90deg, transparent, rgba(255,255,255,0.2), transparent); transition: 0.5s; }
        .btn:hover::before { left: 100%; } .btn:hover { letter-spacing: 2px; } .btn-on:hover { background: var(--primary-dim); box-shadow: 0 0 20px var(--primary); } .btn-off:hover { background: var(--alert-dim); box-shadow: 0 0 20px var(--alert); } .btn:active { transform: scale(0.95); }
        footer { margin-top: 30px; border-top: 1px solid #333; padding-top: 10px; display: flex; justify-content: space-between; font-size: 10px; color: #555; text-transform: uppercase; }
        .blink { animation: blinker 1s linear infinite; }
        @keyframes spin { 100% { transform: rotate(360deg); } } @keyframes pulse { 0% { opacity: 0.8; transform: scale(0.95); } 50% { opacity: 1; transform: scale(1.05); } 100% { opacity: 0.8; transform: scale(0.95); } } @keyframes blinker { 50% { opacity: 0; } }
        @media (max-width: 400px) { .controls { grid-template-columns: 1fr; } h1 { font-size: 18px; } }
    </style>
</head>
<body>
    <div class="interface-container">
        <div class="corner tl"></div><div class="corner tr"></div><div class="corner bl"></div><div class="corner br"></div>
        <header>
            <div><h1>SYSTEM OŚWIETLENIA</h1><div style="font-size: 10px; color: var(--primary);">SEKCJA: POKÓJ JULI</div></div>
            <div class="sys-data"><div id="clock">00:00:00</div><div>NET: <span style="color:var(--primary)">CONNECTED</span></div></div>
        </header>
        <div class="main-display">
            <div class="reactor-core" id="reactor"><div class="reactor-ring"></div><div class="reactor-inner"></div></div>
            <div class="status-text">STATUS: <span id="status-on" class="status-on">WŁĄCZONE</span><span id="status-off" class="status-off">WYŁĄCZONE</span></div>
        </div>
        <div class="controls">
            <button class="btn btn-on" onclick="setPower(true)">AKTYWUJ</button>
            <button class="btn btn-off" onclick="setPower(false)">DEZAKTYWUJ</button>
        </div>
        <footer><div>SYS.VER. 25.08</div><div><span class="blink">▮</span> READY</div></footer>
    </div>
    <script>
        function updateClock() { const now = new Date(); document.getElementById('clock').textContent = now.toLocaleTimeString('pl-PL', { hour12: false }); if(Math.random() > 0.9) document.querySelector('.sys-data div:last-child').style.opacity = Math.random(); }
        setInterval(updateClock, 1000); updateClock();

        function setPower(state) {
            const reactor = document.getElementById('reactor');
            const txtOn = document.getElementById('status-on');
            const txtOff = document.getElementById('status-off');
            if(state) {
                reactor.classList.add('active');
                txtOn.style.display = 'inline'; txtOff.style.display = 'none';
                playSound(440, 'triangle');
                // TUTA WYSYLAMY SYGNAL DO ESP32
                fetch('/on').catch(e => console.log('Blad polaczenia'));
            } else {
                reactor.classList.remove('active');
                txtOn.style.display = 'none'; txtOff.style.display = 'inline';
                playSound(150, 'sawtooth');
                // TUTA WYSYLAMY SYGNAL DO ESP32
                fetch('/off').catch(e => console.log('Blad polaczenia'));
            }
        }

        function playSound(freq, type) {
            try {
                const AudioContext = window.AudioContext || window.webkitAudioContext;
                if (!AudioContext) return;
                const ctx = new AudioContext(); const osc = ctx.createOscillator(); const gain = ctx.createGain();
                osc.type = type; osc.frequency.value = freq; osc.connect(gain); gain.connect(ctx.destination); osc.start();
                gain.gain.setValueAtTime(0.1, ctx.currentTime); gain.gain.exponentialRampToValueAtTime(0.00001, ctx.currentTime + 0.3);
                setTimeout(() => { osc.stop(); }, 300);
            } catch(e) { console.log("Audio blocked"); }
        }
        setPower(false);
    </script>
</body>
</html>
)rawliteral";

/* Func declarations */

// Website handlers
void handleRoot();
void handleLightOn();
void handleLightOff();
void handleNotFound();

// Zigbee
void sendToggleCommand();


void setup() {
  Serial.begin(115200);

  /* Zigbee */

  // Create endpoint
  mySwitch = new ZigbeeSwitch(1);

  // Add to main Zigbee object
  Zigbee.addEndpoint(mySwitch);

  // Start as a coordinator
  if(!Zigbee.begin(ZIGBEE_COORDINATOR)){
    Serial.println("Zigbee start error");
    while(1);
  }

  // Open network for new devices
  Serial.println("Zigbee started");
  Serial.println("Network is open for 3 min - connect your devices");
  Zigbee.setRebootOpenNetwork(180); 

}

void loop() {

}

// Func definitions

// Define the function responsible for sending the Zigbee toggle command
void sendToggleCommand() {
    // Declare a structure variable to hold the On/Off command request parameters
    esp_zb_zcl_on_off_cmd_t cmd_req;
    
    // Zero out the structure memory
    memset(&cmd_req, 0, sizeof(cmd_req));

    // Set the source endpoint to 1
    cmd_req.zcl_basic_cmd.src_endpoint = 1; 
    
    // Set the addressing mode to 16-bit short address with endpoint present
    cmd_req.address_mode = ESP_ZB_APS_ADDR_MODE_16_ENDP_PRESENT; 
    
    // Set the specific command ID to 'Toggle' (switch state)
    cmd_req.on_off_cmd_id = ESP_ZB_ZCL_CMD_ON_OFF_TOGGLE_ID; 
    
    // Set destination address to 0xFFFF (Broadcast to all routers and end devices - we have only one so it works fine)
    cmd_req.zcl_basic_cmd.dst_addr_u.addr_short = 0xFFFF; 
    
    // Set the destination endpoint to 1 (Sonoff devices typically listen on endpoint 1)
    cmd_req.zcl_basic_cmd.dst_endpoint = 1; 
    
    // Print a debug message to the Serial Monitor indicating a broadcast was sent
    Serial.println("Light Toggle (Broadcast)");
    
    // Execute the Zigbee On/Off command request using the configured structure
    esp_zb_zcl_on_off_cmd_req(&cmd_req);
}