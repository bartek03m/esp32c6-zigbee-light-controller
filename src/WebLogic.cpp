#include "WebLogic.h"
#include "Globals.h"
#include "Constants.h"
#include "ZigbeeLogic.h"
#include <WiFi.h>
#include <ESPmDNS.h>
#include <esp_coexist.h>
#include "zcl/esp_zigbee_zcl_common.h"

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
        .sensitivity-panel { margin-top: 25px; border: 1px solid var(--primary-dim); padding: 15px; position: relative; }
        .sensitivity-panel::before { content: 'CZUŁOŚĆ MIKROFONU'; position: absolute; top: -10px; left: 15px; background: var(--panel); padding: 0 8px; font-size: 11px; color: var(--primary); letter-spacing: 2px; }
        .slider-container { display: flex; align-items: center; gap: 15px; margin-top: 10px; }
        .slider-container input[type=range] { flex: 1; -webkit-appearance: none; appearance: none; height: 4px; background: var(--grid); outline: none; border: 1px solid var(--primary-dim); }
        .slider-container input[type=range]::-webkit-slider-thumb { -webkit-appearance: none; appearance: none; width: 18px; height: 18px; background: var(--primary); cursor: pointer; border: none; box-shadow: 0 0 10px var(--primary); }
        .sens-value { font-size: 20px; color: var(--primary); min-width: 60px; text-align: right; text-shadow: 0 0 5px var(--primary); }
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
            <div class="sys-data"><div id="clock">00:00:00</div></div>
        </header>
        <div class="main-display">
            <div class="reactor-core" id="reactor"><div class="reactor-ring"></div><div class="reactor-inner"></div></div>
            <div class="status-text">STATUS: <span id="status-on" class="status-on">WŁĄCZONE</span><span id="status-off" class="status-off">WYŁĄCZONE</span></div>
        </div>
        <div class="controls">
            <button class="btn btn-on" onclick="setPower(true)">AKTYWUJ</button>
            <button class="btn btn-off" onclick="setPower(false)">DEZAKTYWUJ</button>
        </div>
        <div class="sensitivity-panel">
            <div class="slider-container">
                <span style="font-size:11px;color:#555;">WYSOKA</span>
                <input type="range" id="sensSlider" min="2000" max="20000" step="500" value="8500" oninput="updateSens(this.value)" onchange="saveSens(this.value)">
                <span style="font-size:11px;color:#555;">NISKA</span>
                <span class="sens-value" id="sensVal">8500</span>
            </div>
        </div>
        <footer><div>SYS.VER. 25.09</div><div><span class="blink">▮</span></div></footer>
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
                fetch('/on').catch(e => console.log('Blad polaczenia'));
            } else {
                reactor.classList.remove('active');
                txtOn.style.display = 'none'; txtOff.style.display = 'inline';
                playSound(150, 'sawtooth');
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
        function updateUI(state) {
            const reactor = document.getElementById('reactor');
            const txtOn = document.getElementById('status-on');
            const txtOff = document.getElementById('status-off');
            if(state) { reactor.classList.add('active'); txtOn.style.display = 'inline'; txtOff.style.display = 'none'; }
            else { reactor.classList.remove('active'); txtOn.style.display = 'none'; txtOff.style.display = 'inline'; }
        }
        function pollStatus() { fetch('/status').then(r => r.text()).then(v => { updateUI(v === '1'); }).catch(e => {}); }
        pollStatus();
        setInterval(pollStatus, 2000);
        function updateSens(v) { document.getElementById('sensVal').textContent = v; }
        function saveSens(v) { fetch('/sensitivity?val=' + v).catch(e => console.log('Blad')); }
        fetch('/sensitivity').then(r => r.text()).then(v => { document.getElementById('sensSlider').value = v; document.getElementById('sensVal').textContent = v; }).catch(e => console.log('Blad'));
    </script>
</body>
</html>
)rawliteral";

// Initialize WiFi, mDNS and Web Server
void initWiFi()
{
    /* WiFi Website */
    WiFi.mode(WIFI_STA);
    // Serial.println("\n--- SYSTEM START ---");
    // Serial.print("Connecting to WiFi: ");
    // Serial.println(ssid);
    WiFi.begin(ssid, password);

    WiFi.setSleep(false);
    esp_coex_preference_set(ESP_COEX_PREFER_BALANCE);

    Serial.print("[WiFi] Connecting");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.printf("\n[WiFi] Connected! IP: %s\n", WiFi.localIP().toString().c_str());

    // mDNS
    if (MDNS.begin("light"))
    {
        // Serial.println("MDNS: http://light.local");
    }

    server.on("/", handleRoot);
    server.on("/on", handleLightOn);
    server.on("/off", handleLightOff);
    server.on("/sensitivity", handleSensitivity);
    server.on("/status", handleStatus);
    server.onNotFound(handleNotFound);

    // Load saved mic sensitivity from Preferences
    micSensitivity = preferences.getInt("mic_sens", 8500);

    server.begin();
    // Serial.println("Serwer HTTP active");
}

// Handler for root path
void handleRoot()
{
    server.send(200, "text/html", index_html);
}

// Handler for turning light ON
void handleLightOn()
{
    Serial.printf("[WEB] /on request | t=%lus\n", millis() / 1000);
    sendZigbeeCommand(ESP_ZB_ZCL_CMD_ON_OFF_ON_ID);
    lightState = true;
    server.send(200, "text/plain", "OK");
}

// Handler for turning light OFF
void handleLightOff()
{
    Serial.printf("[WEB] /off request | t=%lus\n", millis() / 1000);
    sendZigbeeCommand(ESP_ZB_ZCL_CMD_ON_OFF_OFF_ID);
    lightState = false;
    server.send(200, "text/plain", "OK");
}

// Handler for microphone sensitivity GET/SET
void handleSensitivity()
{
    if (server.hasArg("val"))
    {
        int32_t val = server.arg("val").toInt();
        if (val >= 2000 && val <= 20000)
        {
            micSensitivity = val;
            preferences.putInt("mic_sens", val);
        }
        server.send(200, "text/plain", "OK");
    }
    else
    {
        server.send(200, "text/plain", String(micSensitivity));
    }
}

// Handler for light status
void handleStatus()
{
    server.send(200, "text/plain", lightState ? "1" : "0");
}

// Handler for 404
void handleNotFound()
{
    server.send(404, "text/plain", "404: Not found");
}
