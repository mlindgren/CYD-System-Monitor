#include "web_server.h"
#include "settings_manager.h"
#include <WebServer.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include "esp_timer.h"
#include "esp_system.h"

// Reference the theme variables from config.h
extern const ThemeColors dark_theme;
extern const ThemeColors light_theme;

// Add these near the top of the file
static unsigned long last_micros = 0;
static float cpu_usage = 0;
static const int SAMPLE_COUNT = 10;
static unsigned long samples[SAMPLE_COUNT] = {0};
static int sample_index = 0;

// Improved CPU usage calculation
void updateCPUUsage()
{
    unsigned long current_micros = esp_timer_get_time();

    if (last_micros > 0)
    {
        // Store time between calls
        unsigned long delta = current_micros - last_micros;
        samples[sample_index] = delta;
        sample_index = (sample_index + 1) % SAMPLE_COUNT;

        // Calculate average and max from samples
        unsigned long sum = 0;
        unsigned long max_sample = 0;
        for (int i = 0; i < SAMPLE_COUNT; i++)
        {
            sum += samples[i];
            if (samples[i] > max_sample)
                max_sample = samples[i];
        }
        unsigned long avg = sum / SAMPLE_COUNT;

        // Calculate CPU usage based on processing time
        // Higher deltas mean more CPU time spent
        float usage = (float)avg / max_sample * 100.0;

        // Smooth the transition
        cpu_usage = (cpu_usage * 0.8) + (usage * 0.2);

        // Clamp values
        if (cpu_usage < 0)
            cpu_usage = 0;
        if (cpu_usage > 100)
            cpu_usage = 100;
    }

    last_micros = current_micros;
}

WebServer server(80);

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>ESP32 System Monitor</title>
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <link href="https://cdn.jsdelivr.net/npm/remixicon@3.5.0/fonts/remixicon.css" rel="stylesheet">
    <style>
        :root {
            --bg-color: #fafafa;
            --card-bg: #ffffff;
            --text-color: #020817;
            --primary-color: #0ea5e9;
            --secondary-color: #64748b;
            --success-color: #10b981;
            --danger-color: #ef4444;
            --border-color: #e2e8f0;
            --shadow-color: rgb(0 0 0 / 0.1);
            --radius: 0.5rem;
        }

        [data-theme="dark"] {
            --bg-color: #09090b;
            --card-bg: #111113;
            --text-color: #f8fafc;
            --primary-color: #0ea5e9;
            --secondary-color: #94a3b8;
            --success-color: #10b981;
            --danger-color: #ef4444;
            --border-color: #27272a;
            --shadow-color: rgb(0 0 0 / 0.3);
        }

        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
        }

        body {
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, "Helvetica Neue", sans-serif;
            background: var(--bg-color);
            color: var(--text-color);
            transition: background-color 0.3s ease;
            min-height: 100vh;
            line-height: 1.6;
        }

        .dashboard {
            display: grid;
            grid-template-columns: repeat(12, 1fr);
            grid-template-rows: auto auto auto;
            gap: 1.5rem;
            padding: 1.5rem;
            max-width: 1600px;
            margin: 0 auto;
            width: 100%;
        }

        .header {
            grid-column: span 12;
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding: 1rem 1.5rem;
            background: var(--card-bg);
            border-radius: var(--radius);
            border: 1px solid var(--border-color);
            width: 100%;
        }

        .toolbar {
            grid-column: span 12;
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding: 1rem 1.5rem;
            background: var(--card-bg);
            border-radius: var(--radius);
            border: 1px solid var(--border-color);
            width: 100%;
        }

        .main-content {
            grid-column: span 12;
            display: grid;
            grid-template-columns: repeat(12, 1fr);
            gap: 1.5rem;
            width: 100%;
        }

        .charts-row {
            grid-column: span 12;
            display: grid;
            grid-template-columns: repeat(4, 1fr);
            gap: 1.5rem;
            width: 100%;
        }

        .stats-container {
            grid-column: span 12;
            display: grid;
            grid-template-columns: repeat(5, 1fr);
            gap: 1.5rem;
            width: 100%;
        }

        .card, .chart-card {
            background: var(--card-bg);
            padding: 1.5rem;
            border-radius: var(--radius);
            border: 1px solid var(--border-color);
            transition: all 0.2s ease;
            height: 100%;
            width: 100%;
            overflow: hidden;
        }

        .card:hover, .chart-card:hover {
            box-shadow: 0 4px 6px -1px var(--shadow-color), 0 2px 4px -2px var(--shadow-color);
        }

        .info-grid {
            display: flex;
            flex-direction: column;
            gap: 0.5rem;
            width: 100%;
        }

        .info-item {
            padding: 0.5rem 0.75rem;
            border-radius: var(--radius);
            background: var(--bg-color);
            border: 1px solid var(--border-color);
            width: 100%;
            display: flex;
            justify-content: space-between;
            align-items: center;
            min-height: 2.5rem;
        }

        .info-label {
            font-size: 0.875rem;
            color: var(--secondary-color);
            display: flex;
            align-items: center;
            gap: 0.5rem;
            margin: 0;
            white-space: nowrap;
            overflow: hidden;
            text-overflow: ellipsis;
            flex: 1;
        }

        .info-value {
            font-size: 0.875rem;
            font-weight: 500;
            color: var(--text-color);
            text-align: right;
            white-space: nowrap;
            overflow: hidden;
            text-overflow: ellipsis;
            max-width: 50%;
            padding-left: 1rem;
        }

        .chart-container {
            position: relative;
            height: 150px;
            width: 100%;
        }

        .btn {
            padding: 0.5rem 1rem;
            border-radius: var(--radius);
            font-weight: 500;
            cursor: pointer;
            transition: all 0.2s ease;
            display: inline-flex;
            align-items: center;
            gap: 0.5rem;
            font-size: 0.875rem;
            height: 2.5rem;
            border: 1px solid transparent;
        }

        .btn-success {
            background: var(--success-color);
            color: #ffffff;
            border: none;
        }

        .btn-success:hover {
            opacity: 0.9;
        }

        .btn-danger {
            background: transparent;
            color: var(--danger-color);
            border: 1px solid var(--danger-color);
        }

        .btn-danger:hover {
            background: var(--danger-color);
            color: #ffffff;
        }

        .server-display {
            background: var(--bg-color);
            padding: 0.5rem 0.75rem;
            border-radius: var(--radius);
            border: 1px solid var(--border-color);
            display: flex;
            align-items: center;
            gap: 0.5rem;
            height: 2.5rem;
        }

        .edit-button {
            background: none;
            border: none;
            color: var(--text-color);
            cursor: pointer;
            padding: 0.5rem 0.75rem;
            border-radius: var(--radius);
            display: flex;
            align-items: center;
            gap: 0.5rem;
            font-size: 0.875rem;
            height: 2.5rem;
        }

        .edit-button:hover {
            background: var(--border-color);
        }

        .text-input {
            padding: 0.5rem 0.75rem;
            border-radius: var(--radius);
            border: 1px solid var(--border-color);
            background: var(--bg-color);
            color: var(--text-color);
            font-size: 0.875rem;
            height: 2.5rem;
        }

        .text-input:focus {
            outline: none;
            border-color: var(--primary-color);
            box-shadow: 0 0 0 1px var(--primary-color);
        }

        .server-edit {
            display: none;
            position: relative;
        }

        .server-edit.active {
            display: flex;
            gap: 10px;
        }

        .close-button {
            position: absolute;
            right: -10px;
            top: -10px;
            background: var(--danger-color);
            color: white;
            border: none;
            border-radius: 50%;
            width: 24px;
            height: 24px;
            display: flex;
            align-items: center;
            justify-content: center;
            cursor: pointer;
            box-shadow: 0 2px 4px var(--shadow-color);
        }

        .close-button:hover {
            background: #d32f2f;
        }

        h1 {
            color: var(--text-color);
            font-size: 1.5rem;
            font-weight: 600;
        }

        h2 {
            color: var(--text-color);
            font-size: 1.125rem;
            font-weight: 600;
            margin-bottom: 1rem;
            display: flex;
            align-items: center;
            gap: 0.5rem;
        }

        .gauge-container {
            position: relative;
            display: flex;
            justify-content: center;
            align-items: center;
            padding-bottom: 10px;
        }

        .gauge-value {
            position: absolute;
            bottom: 0;
            font-size: 1.2em;
            font-weight: bold;
            color: var(--text-color);
        }

        .signal-container {
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            gap: 0.5rem;
        }

        .signal-meter {
            display: flex;
            align-items: flex-end;
            gap: 2px;
            height: 80px;
        }

        .signal-bar {
            width: 20px;
            background: var(--border-color);
            border-radius: 3px;
            transition: all 0.3s ease;
            opacity: 0.3;
        }

        .signal-bar:nth-child(1) { height: 25%; }
        .signal-bar:nth-child(2) { height: 50%; }
        .signal-bar:nth-child(3) { height: 75%; }
        .signal-bar:nth-child(4) { height: 100%; }

        .signal-bar.active {
            opacity: 1;
        }

        .signal-value {
            font-size: 1.2em;
            font-weight: bold;
            color: var(--text-color);
        }

        .theme-controls {
            display: grid;
            gap: 15px;
            flex: 1;
        }

        .theme-controls .btn {
            margin-top: auto;
        }

        .color-picker {
            display: flex;
            align-items: center;
            justify-content: space-between;
            gap: 10px;
        }
        
        .color-picker label {
            flex: 1;
        }
        
        .color-picker input[type="color"] {
            width: 50px;
            height: 30px;
            padding: 0;
            border: none;
            border-radius: 4px;
            cursor: pointer;
        }

        @media (max-width: 1400px) {
            .stats-container {
                grid-template-columns: repeat(2, 1fr);
            }
            .stats-container .system-card,
            .stats-container .memory-card,
            .stats-container .theme-card {
                grid-column: span 1;
            }
        }

        @media (max-width: 768px) {
            .dashboard {
                height: auto;
            }
            .charts-row {
                grid-template-columns: 1fr;
            }
            .stats-container {
                grid-template-columns: 1fr;
            }
            .toolbar {
                flex-direction: column;
                align-items: flex-start;
                gap: 10px;
            }
            .glances-settings {
                flex-direction: column;
                align-items: flex-start;
                width: 100%;
            }
            .text-input {
                width: 100%;
            }
        }

        .theme-toggle {
            display: flex;
            align-items: center;
            gap: 0.5rem;
            color: var(--text-color);
        }

        .switch {
            position: relative;
            display: inline-block;
            width: 48px;
            height: 24px;
        }

        .switch input {
            opacity: 0;
            width: 0;
            height: 0;
        }

        .slider {
            position: absolute;
            cursor: pointer;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
            background-color: var(--border-color);
            transition: .3s;
            border-radius: 24px;
            border: 1px solid var(--border-color);
        }

        .slider:before {
            position: absolute;
            content: "";
            height: 18px;
            width: 18px;
            left: 2px;
            bottom: 2px;
            background-color: var(--card-bg);
            transition: .3s;
            border-radius: 50%;
            border: 1px solid var(--border-color);
            top: 50%;
            transform: translateY(-50%);
        }

        input:checked + .slider {
            background-color: var(--primary-color);
        }

        input:checked + .slider:before {
            transform: translate(24px, -50%);
            background-color: var(--card-bg);
        }

        .footer {
            grid-column: span 12;
            text-align: center;
            padding: 1rem;
            color: var(--secondary-color);
            font-size: 0.875rem;
        }

        .footer a {
            color: var(--primary-color);
            text-decoration: none;
            display: inline-flex;
            align-items: center;
            gap: 0.5rem;
        }

        .footer a:hover {
            text-decoration: underline;
        }
    </style>
</head>
<body>
    <div class="dashboard">
        <header class="header">
            <h1><i class="ri-cpu-line"></i> ESP32 System Monitor</h1>
            <div class="theme-toggle">
                <span><i class="ri-sun-line"></i></span>
                <label class="switch">
                    <input type="checkbox" id="darkMode" onchange="updateTheme()">
                    <span class="slider"></span>
                </label>
                <span><i class="ri-moon-line"></i></span>
            </div>
        </header>

        <div class="toolbar">
            <div class="glances-settings">
                <div class="server-info">
                    <div class="server-display">
                        <i class="ri-server-line"></i>
                        <span id="serverDisplay">Not configured</span>
                        <button class="edit-button" onclick="toggleServerEdit()">
                            <i class="ri-edit-line"></i>
                            Edit Glances Server
                        </button>
                    </div>
                    <div class="server-edit">
                        <button class="close-button" onclick="toggleServerEdit()">
                            <i class="ri-close-line"></i>
                        </button>
                        <input type="text" id="glancesHost" class="text-input" placeholder="Glances IP">
                        <input type="number" id="glancesPort" class="text-input" placeholder="Port">
                        <button class="btn btn-success" onclick="saveGlancesSettings()">
                            <i class="ri-save-line"></i> Save
                        </button>
                    </div>
                </div>
            </div>
            <button class="btn btn-danger" onclick="restartESP()">
                <i class="ri-restart-line"></i> Restart ESP32
            </button>
        </div>

        <div class="main-content">
            <div class="charts-row">
                <div class="chart-card">
                    <h2><i class="ri-cpu-line"></i> CPU Usage</h2>
                    <div class="chart-container">
                        <canvas id="cpuChart"></canvas>
                    </div>
                </div>
                <div class="chart-card">
                    <h2><i class="ri-database-2-line"></i> Memory</h2>
                    <div class="chart-container">
                        <canvas id="memoryChart"></canvas>
                    </div>
                </div>
                <div class="chart-card">
                    <h2><i class="ri-temp-hot-line"></i> Temperature</h2>
                    <div class="chart-container gauge-container">
                        <canvas id="tempGauge"></canvas>
                        <div class="gauge-value" id="tempValue">--°C</div>
                    </div>
                </div>
                <div class="chart-card">
                    <h2><i class="ri-wifi-line"></i> WiFi Signal</h2>
                    <div class="chart-container signal-container">
                        <div class="signal-meter">
                            <div class="signal-bar"></div>
                            <div class="signal-bar"></div>
                            <div class="signal-bar"></div>
                            <div class="signal-bar"></div>
                        </div>
                        <div class="signal-value" id="signalValue">-- dBm</div>
                    </div>
                </div>
            </div>

            <div class="stats-container">
                <div class="card system-card">
                    <h2><i class="ri-information-line"></i> System Info</h2>
                    <div class="info-grid">
                        <div class="info-item">
                            <div class="info-label"><i class="ri-cpu-line"></i> Model</div>
                            <div class="info-value" id="chipModel">---</div>
                        </div>
                        <div class="info-item">
                            <div class="info-label"><i class="ri-cpu-line"></i> Revision</div>
                            <div class="info-value" id="chipRevision">---</div>
                        </div>
                        <div class="info-item">
                            <div class="info-label"><i class="ri-code-s-slash-line"></i> SDK Version</div>
                            <div class="info-value" id="sdkVersion">---</div>
                        </div>
                        <div class="info-item">
                            <div class="info-label"><i class="ri-speed-line"></i> CPU Freq</div>
                            <div class="info-value" id="cpuFreqMHz">---</div>
                        </div>
                        <div class="info-item">
                            <div class="info-label"><i class="ri-refresh-line"></i> Cycle Count</div>
                            <div class="info-value" id="cycleCount">---</div>
                        </div>
                        <div class="info-item">
                            <div class="info-label"><i class="ri-fingerprint-line"></i> Efuse MAC</div>
                            <div class="info-value" id="efuseMac">---</div>
                        </div>
                        <div class="info-item">
                            <div class="info-label"><i class="ri-temp-hot-line"></i> Temperature</div>
                            <div class="info-value" id="temperature">---</div>
                        </div>
                        <div class="info-item">
                            <div class="info-label"><i class="ri-sensor-line"></i> Hall Sensor</div>
                            <div class="info-value" id="hallSensor">---</div>
                        </div>
                        <div class="info-item">
                            <div class="info-label"><i class="ri-time-line"></i> Uptime</div>
                            <div class="info-value" id="uptime">---</div>
                        </div>
                        <div class="info-item">
                            <div class="info-label"><i class="ri-error-warning-line"></i> Last Reset</div>
                            <div class="info-value" id="lastResetReason">---</div>
                        </div>
                    </div>
                </div>

                <div class="card memory-card">
                    <h2><i class="ri-database-2-line"></i> Memory Info</h2>
                    <div class="info-grid">
                        <div class="info-item">
                            <div class="info-label"><i class="ri-database-2-line"></i> Total Heap</div>
                            <div class="info-value" id="totalHeap">---</div>
                        </div>
                        <div class="info-item">
                            <div class="info-label"><i class="ri-database-2-line"></i> Free Heap</div>
                            <div class="info-value" id="freeHeap">---</div>
                        </div>
                        <div class="info-item">
                            <div class="info-label"><i class="ri-database-2-line"></i> Min Free Heap</div>
                            <div class="info-value" id="minFreeHeap">---</div>
                        </div>
                        <div class="info-item">
                            <div class="info-label"><i class="ri-database-2-line"></i> Max Alloc Heap</div>
                            <div class="info-value" id="maxAllocHeap">---</div>
                        </div>
                        <div class="info-item">
                            <div class="info-label"><i class="ri-split-cells-horizontal"></i> Fragmentation</div>
                            <div class="info-value" id="heapFragmentation">---</div>
                        </div>
                        <div class="info-item">
                            <div class="info-label"><i class="ri-database-2-line"></i> PSRAM Size</div>
                            <div class="info-value" id="psramSize">---</div>
                        </div>
                        <div class="info-item">
                            <div class="info-label"><i class="ri-database-2-line"></i> Free PSRAM</div>
                            <div class="info-value" id="freePsram">---</div>
                        </div>
                        <div class="info-item">
                            <div class="info-label"><i class="ri-database-2-line"></i> Min Free PSRAM</div>
                            <div class="info-value" id="minFreePsram">---</div>
                        </div>
                        <div class="info-item">
                            <div class="info-label"><i class="ri-database-2-line"></i> Max Alloc PSRAM</div>
                            <div class="info-value" id="maxAllocPsram">---</div>
                        </div>
                    </div>
                </div>
                                <div class="card theme-card">
                    <h2><i class="ri-palette-line"></i> Theme Customization</h2>
                    <div class="theme-controls">
                        <div class="color-picker">
                            <label>Background</label>
                            <input type="color" id="bgColor" onchange="updateThemeColor('bg_color', this.value)">
                        </div>
                        <div class="color-picker">
                            <label>Card Background</label>
                            <input type="color" id="cardBgColor" onchange="updateThemeColor('card_bg_color', this.value)">
                        </div>
                        <div class="color-picker">
                            <label>Text</label>
                            <input type="color" id="textColor" onchange="updateThemeColor('text_color', this.value)">
                        </div>
                        <div class="color-picker">
                            <label>CPU Ring</label>
                            <input type="color" id="cpuColor" onchange="updateThemeColor('cpu_color', this.value)">
                        </div>
                        <div class="color-picker">
                            <label>RAM Ring</label>
                            <input type="color" id="ramColor" onchange="updateThemeColor('ram_color', this.value)">
                        </div>
                        <div class="color-picker">
                            <label>Border</label>
                            <input type="color" id="borderColor" onchange="updateThemeColor('border_color', this.value)">
                        </div>
        
                        <button class="btn btn-success" onclick="resetTheme()">
                            <i class="ri-refresh-line"></i> Reset Theme
                        </button>
                    </div>
                </div>
                <div class="card flash-card">
                    <h2><i class="ri-sd-card-line"></i> Flash Info</h2>
                    <div class="info-grid">
                        <div class="info-item">
                            <div class="info-label"><i class="ri-sd-card-line"></i> Flash Size</div>
                            <div class="info-value" id="flashChipSize">---</div>
                        </div>
                        <div class="info-item">
                            <div class="info-label"><i class="ri-speed-line"></i> Flash Speed</div>
                            <div class="info-value" id="flashChipSpeed">---</div>
                        </div>
                        <div class="info-item">
                            <div class="info-label"><i class="ri-settings-line"></i> Flash Mode</div>
                            <div class="info-value" id="flashChipMode">---</div>
                        </div>
                        <div class="info-item">
                            <div class="info-label"><i class="ri-file-code-line"></i> Sketch Size</div>
                            <div class="info-value" id="sketchSize">---</div>
                        </div>
                        <div class="info-item">
                            <div class="info-label"><i class="ri-fingerprint-line"></i> Sketch MD5</div>
                            <div class="info-value" id="sketchMD5">---</div>
                        </div>
                        <div class="info-item">
                            <div class="info-label"><i class="ri-hard-drive-line"></i> Free Space</div>
                            <div class="info-value" id="freeSketchSpace">---</div>
                        </div>
                    </div>
                </div>
                <div class="card network-card">
                    <h2><i class="ri-wifi-line"></i> Network Info</h2>
                    <div class="info-grid">
                        <div class="info-item">
                            <div class="info-label"><i class="ri-wifi-line"></i> SSID</div>
                            <div class="info-value" id="wifiSSID">---</div>
                        </div>
                        <div class="info-item">
                            <div class="info-label"><i class="ri-broadcast-line"></i> BSSID</div>
                            <div class="info-value" id="wifiBSSID">---</div>
                        </div>
                        <div class="info-item">
                            <div class="info-label"><i class="ri-eye-off-line"></i> Hidden</div>
                            <div class="info-value" id="isHidden">---</div>
                        </div>
                        <div class="info-item">
                            <div class="info-label"><i class="ri-refresh-line"></i> Auto Reconnect</div>
                            <div class="info-value" id="autoReconnect">---</div>
                        </div>
                        <div class="info-item">
                            <div class="info-label"><i class="ri-computer-line"></i> Hostname</div>
                            <div class="info-value" id="hostname">---</div>
                        </div>
                        <div class="info-item">
                            <div class="info-label"><i class="ri-wifi-line"></i> WiFi Mode</div>
                            <div class="info-value" id="wifiMode">---</div>
                        </div>
                        <div class="info-item">
                            <div class="info-label"><i class="ri-ip-fill"></i> IP Address</div>
                            <div class="info-value" id="ipAddress">---</div>
                        </div>
                        <div class="info-item">
                            <div class="info-label"><i class="ri-mac-line"></i> MAC Address</div>
                            <div class="info-value" id="macAddress">---</div>
                        </div>
                        <div class="info-item">
                            <div class="info-label"><i class="ri-global-line"></i> DNS Server</div>
                            <div class="info-value" id="dnsIP">---</div>
                        </div>
                        <div class="info-item">
                            <div class="info-label"><i class="ri-router-line"></i> Gateway</div>
                            <div class="info-value" id="gatewayIP">---</div>
                        </div>
                        <div class="info-item">
                            <div class="info-label"><i class="ri-filter-line"></i> Subnet Mask</div>
                            <div class="info-value" id="subnetMask">---</div>
                        </div>
                    </div>
                </div>
            </div>
        </div>
    </div>

    <div class="footer">
        <a href="https://github.com/iamlite/ESP32-Yellow-Display-System-Monitor" target="_blank">
            <i class="ri-github-line"></i> View on GitHub
        </a>
    </div>

    <script>
        // Add data arrays for the charts
        const maxDataPoints = 30;
        const cpuData = Array(maxDataPoints).fill(0);
        const memoryData = Array(maxDataPoints).fill(0);
        const labels = Array(maxDataPoints).fill('');

        // Initialize line charts
        const cpuChart = new Chart(document.getElementById('cpuChart').getContext('2d'), {
            type: 'line',
            data: {
                labels: labels,
                datasets: [{
                    label: 'CPU Usage %',
                    data: cpuData,
                    borderColor: getComputedStyle(document.documentElement).getPropertyValue('--primary-color'),
                    tension: 0.4,
                    fill: false
                }]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                scales: {
                    y: {
                        beginAtZero: true,
                        max: 100,
                        ticks: {
                            color: getComputedStyle(document.documentElement).getPropertyValue('--text-color')
                        },
                        grid: {
                            color: getComputedStyle(document.documentElement).getPropertyValue('--border-color')
                        }
                    },
                    x: {
                        display: false
                    }
                },
                plugins: {
                    legend: {
                        display: false
                    }
                }
            }
        });

        const memoryChart = new Chart(document.getElementById('memoryChart').getContext('2d'), {
            type: 'line',
            data: {
                labels: labels,
                datasets: [{
                    label: 'Memory Usage %',
                    data: memoryData,
                    borderColor: getComputedStyle(document.documentElement).getPropertyValue('--success-color'),
                    tension: 0.4,
                    fill: false
                }]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                scales: {
                    y: {
                        beginAtZero: true,
                        max: 100,
                        ticks: {
                            color: getComputedStyle(document.documentElement).getPropertyValue('--text-color')
                        },
                        grid: {
                            color: getComputedStyle(document.documentElement).getPropertyValue('--border-color')
                        }
                    },
                    x: {
                        display: false
                    }
                },
                plugins: {
                    legend: {
                        display: false
                    }
                }
            }
        });

        // Initialize theme state from server
        fetch('/settings')
            .then(response => response.json())
            .then(data => {
                const darkMode = data.darkMode;
                document.getElementById('darkMode').checked = darkMode;
                document.documentElement.setAttribute('data-theme', darkMode ? 'dark' : 'light');
                
                // Initialize Glances settings
                document.getElementById('glancesHost').value = data.glances_host;
                document.getElementById('glancesPort').value = data.glances_port;
                updateServerDisplay();
            });

        // Initialize gauge charts with modified options
        const gaugeOptions = {
            type: 'doughnut',
            options: {
                responsive: true,
                maintainAspectRatio: false,
                cutout: '75%',
                plugins: {
                    legend: { display: false }
                },
                elements: {
                    arc: {
                        borderWidth: 0  // Remove the border
                    }
                }
            }
        };

        const tempGauge = new Chart(document.getElementById('tempGauge').getContext('2d'), {
            ...gaugeOptions,
            data: {
                datasets: [{
                    data: [0, 100],
                    backgroundColor: [
                        getComputedStyle(document.documentElement).getPropertyValue('--danger-color'),
                        getComputedStyle(document.documentElement).getPropertyValue('--border-color')
                    ],
                    circumference: 180,
                    rotation: 270
                }]
            }
        });

        function updateSignalStrength(dbm) {
            const bars = document.querySelectorAll('.signal-bar');
            const value = document.getElementById('signalValue');
            value.textContent = dbm + ' dBm';

            // Convert dBm to quality percentage
            // Typical WiFi range is -50 dBm (excellent) to -90 dBm (poor)
            const quality = Math.max(0, Math.min(100, (dbm + 90) * 2.5));
            
            bars.forEach((bar, index) => {
                const threshold = (index + 1) * 25;
                bar.classList.toggle('active', quality >= threshold);
            });
        }

        function updateVisualizations(data) {
            // Update CPU chart
            cpuData.push(parseFloat(data.cpuUsage));
            cpuData.shift();
            cpuChart.update();

            // Update memory chart
            const totalHeap = parseFloat(data.totalHeap);
            const freeHeap = parseFloat(data.freeHeap);
            const usedHeap = totalHeap - freeHeap;
            const memoryUsage = Math.max(0, Math.min(100, (usedHeap / totalHeap) * 100));
            memoryData.push(memoryUsage);
            memoryData.shift();
            memoryChart.update();

            // Update temperature gauge
            const temp = parseFloat(data.temperature);
            tempGauge.data.datasets[0].data = [temp, 100 - temp];
            tempGauge.update();
            document.getElementById('tempValue').textContent = `${temp.toFixed(1)}°C`;

            // Update WiFi signal strength
            updateSignalStrength(data.wifiStrength);
        }

        function updateTheme() {
            const darkMode = document.getElementById('darkMode').checked;
            document.documentElement.setAttribute('data-theme', darkMode ? 'dark' : 'light');
            
            fetch('/settings', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify({ darkMode })
            });
        }

        function formatUptime(seconds) {
            const days = Math.floor(seconds / 86400);
            const hours = Math.floor((seconds % 86400) / 3600);
            const minutes = Math.floor((seconds % 3600) / 60);
            const secs = seconds % 60;
            
            if (days > 0) return `${days}d ${hours}h ${minutes}m`;
            if (hours > 0) return `${hours}h ${minutes}m ${secs}s`;
            if (minutes > 0) return `${minutes}m ${secs}s`;
            return `${secs}s`;
        }

        function updateThemeColor(property, value) {
            const darkMode = document.getElementById('darkMode').checked;
            const color = parseInt(value.substring(1), 16);

            fetch('/settings', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify({ 
                    darkMode: darkMode,
                    [property]: color 
                })
            }).then(() => {
                setTimeout(updateColorPickers, 100);
            });
        }

        function updateColorPickers() {
            fetch('/settings')
                .then(response => response.json())
                .then(data => {
                    const pickers = {
                        'bg_color': 'bgColor',
                        'card_bg_color': 'cardBgColor',
                        'text_color': 'textColor',
                        'cpu_color': 'cpuColor',
                        'ram_color': 'ramColor',
                        'border_color': 'borderColor'
                    };
                    
                    Object.entries(pickers).forEach(([key, id]) => {
                        const picker = document.getElementById(id);
                        if (picker) {
                            picker.value = data[id];
                        }
                    });
                });
        }

        function resetTheme() {
            fetch('/resetTheme', {
                method: 'POST',
            }).then(response => response.json())
            .then(data => {
                if (data.status === 'success') {
                    window.location.reload();
                }
            });
        }

        function restartESP() {
            if (confirm('Are you sure you want to restart the ESP32?')) {
                fetch('/restart', { method: 'POST' })
                    .then(() => {
                        alert('ESP32 is restarting...');
                        setTimeout(() => {
                            window.location.reload();
                        }, 5000);
                    });
            }
        }

        function updateSystemInfo(data) {
            // Update all the new fields
            Object.keys(data).forEach(key => {
                const element = document.getElementById(key);
                if (element) {
                    let value = data[key];
                    
                    // Add units where appropriate
                    if (key === 'cpuFreqMHz') value += ' MHz';
                    if (key === 'flashChipSpeed') value += ' MHz';
                    if (key.includes('Size') || key.includes('Heap') || key.includes('Space')) value += ' KB';
                    if (key === 'temperature') value += ' °C';
                    if (key === 'wifiStrength') value += ' dBm';
                    if (key === 'heapFragmentation') value += '%';
                    
                    element.textContent = value;
                }
            });
        }

        let isEditing = false;

        function toggleServerEdit() {
            const displayEl = document.querySelector('.server-display');
            const editEl = document.querySelector('.server-edit');
            editEl.classList.toggle('active');
            displayEl.style.display = editEl.classList.contains('active') ? 'none' : 'flex';
            isEditing = editEl.classList.contains('active');
        }

        function updateServerDisplay() {
            const host = document.getElementById('glancesHost').value;
            const port = document.getElementById('glancesPort').value;
            const displayEl = document.getElementById('serverDisplay');
            
            if (host && port) {
                displayEl.textContent = `${host}:${port}`;
            } else {
                displayEl.textContent = 'Not configured';
            }
        }

        // Modify the existing interval fetch
        setInterval(() => {
            fetch('/settings')
                .then(response => response.json())
                .then(data => {
                    updateSystemInfo(data);
                    updateVisualizations(data);
                    updateColorPickers();
                    
                    if (!isEditing) {
                        document.getElementById('glancesHost').value = data.glances_host;
                        document.getElementById('glancesPort').value = data.glances_port;
                        updateServerDisplay();
                    }
                });
        }, 2000);

        // Modify the saveGlancesSettings function
        function saveGlancesSettings() {
            const host = document.getElementById('glancesHost').value;
            const port = parseInt(document.getElementById('glancesPort').value);
            
            if (!host || !port) {
                alert('Please enter both host and port');
                return;
            }
            
            fetch('/settings', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify({
                    glances_host: host,
                    glances_port: port
                })
            })
            .then(response => response.json())
            .then(data => {
                if (data.status === 'success') {
                    updateServerDisplay();
                    toggleServerEdit();
                    alert('Glances settings updated successfully!');
                }
            })
            .catch(error => {
                console.error('Error:', error);
                alert('Failed to update Glances settings');
            });
        }
    </script>
</body>
</html>
)rawliteral";

void handleRoot()
{
    server.send(200, "text/html", INDEX_HTML);
}

void handleGetSettings()
{
    StaticJsonDocument<1024> doc;

    // Update CPU usage
    updateCPUUsage();
    doc["cpuUsage"] = (int)cpu_usage; // Cast to int to ensure clean number

    // Make sure we keep the WiFi strength for the gauge display
    doc["wifiStrength"] = WiFi.RSSI();

    // Extended System Information
    doc["chipModel"] = ESP.getChipModel();
    doc["chipRevision"] = ESP.getChipRevision();
    doc["sdkVersion"] = ESP.getSdkVersion();
    doc["cpuFreqMHz"] = ESP.getCpuFreqMHz();
    doc["cycleCount"] = ESP.getCycleCount();
    doc["efuseMac"] = ESP.getEfuseMac();
    doc["temperature"] = String((temperatureRead() - 32) / 1.8, 2);
    doc["hallSensor"] = hallRead();
    doc["uptime"] = millis() / 1000;
    doc["lastResetReason"] = esp_reset_reason();

    // Calculate heap fragmentation manually
    uint32_t free_heap = ESP.getFreeHeap();
    uint32_t max_alloc = ESP.getMaxAllocHeap();
    uint32_t fragmentation = 100 - (max_alloc * 100) / free_heap;

    // Extended Memory Information
    doc["totalHeap"] = ESP.getHeapSize() / 1024;
    doc["freeHeap"] = free_heap / 1024;
    doc["minFreeHeap"] = ESP.getMinFreeHeap() / 1024;
    doc["maxAllocHeap"] = max_alloc / 1024;
    doc["heapFragmentation"] = fragmentation;
    doc["psramSize"] = ESP.getPsramSize() / 1024;
    doc["freePsram"] = ESP.getFreePsram() / 1024;
    doc["minFreePsram"] = ESP.getMinFreePsram() / 1024;
    doc["maxAllocPsram"] = ESP.getMaxAllocPsram() / 1024;

    // Flash Information
    doc["flashChipSize"] = ESP.getFlashChipSize() / 1024;
    doc["flashChipSpeed"] = ESP.getFlashChipSpeed() / 1000000;
    doc["flashChipMode"] = ESP.getFlashChipMode();
    doc["sketchSize"] = ESP.getSketchSize() / 1024;
    doc["sketchMD5"] = ESP.getSketchMD5();
    doc["freeSketchSpace"] = ESP.getFreeSketchSpace() / 1024;

    // Extended WiFi Information
    doc["wifiSSID"] = WiFi.SSID();
    doc["wifiBSSID"] = WiFi.BSSIDstr();
    doc["ipAddress"] = WiFi.localIP().toString();
    doc["macAddress"] = WiFi.macAddress();
    doc["channel"] = WiFi.channel();
    doc["wifiTxPower"] = WiFi.getTxPower();
    doc["dnsIP"] = WiFi.dnsIP().toString();
    doc["gatewayIP"] = WiFi.gatewayIP().toString();
    doc["subnetMask"] = WiFi.subnetMask().toString();
    doc["hostname"] = WiFi.getHostname();
    doc["wifiMode"] = WiFi.getMode();
    doc["isHidden"] = WiFi.SSID().length() == 0;
    doc["autoReconnect"] = WiFi.getAutoReconnect();

    // Theme Colors
    const ThemeColors &theme = SettingsManager::getCurrentTheme();
    char hexColor[8];
    uint32_t color;

    color = lv_color_to32(theme.bg_color);
    sprintf(hexColor, "#%02X%02X%02X",
            (uint8_t)((color >> 16) & 0xFF),
            (uint8_t)((color >> 8) & 0xFF),
            (uint8_t)(color & 0xFF));
    doc["bgColor"] = hexColor;

    color = lv_color_to32(theme.text_color);
    sprintf(hexColor, "#%02X%02X%02X",
            (uint8_t)((color >> 16) & 0xFF),
            (uint8_t)((color >> 8) & 0xFF),
            (uint8_t)(color & 0xFF));
    doc["textColor"] = hexColor;

    color = lv_color_to32(theme.cpu_color);
    sprintf(hexColor, "#%02X%02X%02X",
            (uint8_t)((color >> 16) & 0xFF),
            (uint8_t)((color >> 8) & 0xFF),
            (uint8_t)(color & 0xFF));
    doc["cpuColor"] = hexColor;

    color = lv_color_to32(theme.ram_color);
    sprintf(hexColor, "#%02X%02X%02X",
            (uint8_t)((color >> 16) & 0xFF),
            (uint8_t)((color >> 8) & 0xFF),
            (uint8_t)(color & 0xFF));
    doc["ramColor"] = hexColor;

    color = lv_color_to32(theme.border_color);
    sprintf(hexColor, "#%02X%02X%02X",
            (uint8_t)((color >> 16) & 0xFF),
            (uint8_t)((color >> 8) & 0xFF),
            (uint8_t)(color & 0xFF));
    doc["borderColor"] = hexColor;

    color = lv_color_to32(theme.card_bg_color);
    sprintf(hexColor, "#%02X%02X%02X",
            (uint8_t)((color >> 16) & 0xFF),
            (uint8_t)((color >> 8) & 0xFF),
            (uint8_t)(color & 0xFF));
    doc["cardBgColor"] = hexColor;

    doc["darkMode"] = SettingsManager::getDarkMode();

    // Add these lines to the existing function
    doc["glances_host"] = SettingsManager::getGlancesHost();
    doc["glances_port"] = SettingsManager::getGlancesPort();

    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
}

void handleUpdateSettings()
{
    String json = server.arg("plain");
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, json);

    if (!error)
    {
        if (doc.containsKey("darkMode"))
        {
            SettingsManager::setDarkMode(doc["darkMode"].as<bool>());
        }
        if (doc.containsKey("bg_color"))
        {
            uint32_t color = doc["bg_color"].as<uint32_t>();
            Serial.printf("Updating bg_color to: %06X\n", color);
            SettingsManager::updateThemeColor("bg_color", color);
        }
        if (doc.containsKey("text_color"))
        {
            SettingsManager::updateThemeColor("text_color", doc["text_color"].as<uint32_t>());
        }
        if (doc.containsKey("cpu_color"))
        {
            SettingsManager::updateThemeColor("cpu_color", doc["cpu_color"].as<uint32_t>());
        }
        if (doc.containsKey("ram_color"))
        {
            SettingsManager::updateThemeColor("ram_color", doc["ram_color"].as<uint32_t>());
        }
        if (doc.containsKey("border_color"))
        {
            SettingsManager::updateThemeColor("border_color", doc["border_color"].as<uint32_t>());
        }
        if (doc.containsKey("card_bg_color"))
        {
            SettingsManager::updateThemeColor("card_bg_color", doc["card_bg_color"].as<uint32_t>());
        }
        // Add these conditions to the existing if block
        if (doc.containsKey("glances_host"))
        {
            SettingsManager::setGlancesHost(doc["glances_host"].as<String>());
        }
        if (doc.containsKey("glances_port"))
        {
            SettingsManager::setGlancesPort(doc["glances_port"].as<uint16_t>());
        }
        server.send(200, "application/json", "{\"status\":\"success\"}");
    }
    else
    {
        server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid JSON\"}");
    }
}

void handleRestart()
{
    server.send(200, "application/json", "{\"status\":\"success\"}");
    delay(500);
    ESP.restart();
}

void handleResetTheme()
{
    // Reset to original theme colors
    if (SettingsManager::getDarkMode())
    {
        const_cast<ThemeColors &>(SettingsManager::getCurrentTheme()) = dark_theme;
    }
    else
    {
        const_cast<ThemeColors &>(SettingsManager::getCurrentTheme()) = light_theme;
    }

    // Clear saved colors from preferences
    SettingsManager::clearSavedColors();

    // Update the display
    if (SettingsManager::themeCallback)
    {
        SettingsManager::themeCallback(SettingsManager::getDarkMode());
    }

    server.send(200, "application/json", "{\"status\":\"success\"}");
}

// Add these new REST API endpoints for Home Assistant
void handleHaStatus()
{
    StaticJsonDocument<256> doc;

    // Basic system stats that HA might want to monitor
    doc["temperature"] = String((temperatureRead() - 32) / 1.8, 2);
    doc["free_heap"] = ESP.getFreeHeap() / 1024;
    doc["wifi_strength"] = WiFi.RSSI();
    doc["uptime"] = millis() / 1000;
    doc["dark_mode"] = SettingsManager::getDarkMode();

    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
}

void handleHaCommand()
{
    if (server.method() != HTTP_POST)
    {
        server.send(405, "application/json", "{\"error\":\"Method not allowed\"}");
        return;
    }

    String json = server.arg("plain");
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, json);

    if (error)
    {
        server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        return;
    }

    bool success = false;
    String message = "Unknown command";

    if (doc.containsKey("dark_mode"))
    {
        SettingsManager::setDarkMode(doc["dark_mode"].as<bool>());
        success = true;
        message = "Dark mode updated";
    }

    if (doc.containsKey("restart"))
    {
        if (doc["restart"].as<bool>())
        {
            success = true;
            message = "Restarting device";
            // Schedule restart after sending response
            server.send(200, "application/json", "{\"success\":true,\"message\":\"Restarting device\"}");
            delay(500);
            ESP.restart();
            return;
        }
    }

    if (doc.containsKey("reset_theme"))
    {
        if (doc["reset_theme"].as<bool>())
        {
            // Reset theme to defaults
            if (SettingsManager::getDarkMode())
            {
                const_cast<ThemeColors &>(SettingsManager::getCurrentTheme()) = dark_theme;
            }
            else
            {
                const_cast<ThemeColors &>(SettingsManager::getCurrentTheme()) = light_theme;
            }
            SettingsManager::clearSavedColors();
            if (SettingsManager::themeCallback)
            {
                SettingsManager::themeCallback(SettingsManager::getDarkMode());
            }
            success = true;
            message = "Theme reset to defaults";
        }
    }

    StaticJsonDocument<128> response;
    response["success"] = success;
    response["message"] = message;

    String responseStr;
    serializeJson(response, responseStr);
    server.send(200, "application/json", responseStr);
}

void setupWebServer()
{
    server.on("/", HTTP_GET, handleRoot);
    server.on("/settings", HTTP_GET, handleGetSettings);
    server.on("/settings", HTTP_POST, handleUpdateSettings);
    server.on("/restart", HTTP_POST, handleRestart);
    server.on("/resetTheme", HTTP_POST, handleResetTheme);

    // Add new REST API endpoints for Home Assistant
    server.on("/api/status", HTTP_GET, handleHaStatus);
    server.on("/api/command", HTTP_POST, handleHaCommand);

    server.begin();
}

void handleWebServer()
{
    server.handleClient();
}