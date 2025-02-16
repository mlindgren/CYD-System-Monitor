#include "web_server.h"
#include "settings_manager.h"
#include <WebServer.h>
#include <ArduinoJson.h>
#include <WiFi.h>

// Reference the theme variables from config.h
extern const ThemeColors dark_theme;
extern const ThemeColors light_theme;

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
            --bg-color: #f0f0f0;
            --card-bg: #ffffff;
            --text-color: #333333;
            --primary-color: #2196F3;
            --secondary-color: #1976D2;
            --success-color: #4CAF50;
            --danger-color: #f44336;
            --border-color: rgba(0,0,0,0.1);
            --shadow-color: rgba(0,0,0,0.1);
        }

        [data-theme="dark"] {
            --bg-color: #121212;
            --card-bg: #1E1E1E;
            --text-color: #ffffff;
            --primary-color: #64B5F6;
            --secondary-color: #90CAF9;
            --success-color: #81C784;
            --danger-color: #E57373;
            --border-color: rgba(255,255,255,0.1);
            --shadow-color: rgba(0,0,0,0.3);
        }

        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
        }

        body {
            font-family: 'Segoe UI', Arial, sans-serif;
            background: var(--bg-color);
            color: var(--text-color);
            transition: all 0.3s ease;
            min-height: 100vh;
            line-height: 1.6;
        }

        .dashboard {
            display: grid;
            grid-template-columns: repeat(12, 1fr);
            grid-template-rows: auto auto auto;
            gap: 20px;
            padding: 20px;
            max-width: 1600px;
            margin: 0 auto;
        }

        .header {
            grid-column: span 12;
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding: 10px 20px;
            background: var(--card-bg);
            border-radius: 12px;
            box-shadow: 0 4px 6px var(--shadow-color);
        }

        .toolbar {
            grid-column: span 12;
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding: 10px 20px;
            background: var(--card-bg);
            border-radius: 12px;
            box-shadow: 0 4px 6px var(--shadow-color);
        }

        .main-content {
            grid-column: span 12;
            display: grid;
            grid-template-columns: repeat(12, 1fr);
            gap: 20px;
            overflow-y: auto;
        }

        .charts-row {
            grid-column: span 12;
            display: grid;
            grid-template-columns: repeat(3, 1fr);
            gap: 20px;
        }

        .stats-container {
            grid-column: span 9;
            display: grid;
            grid-template-columns: repeat(3, 1fr);
            gap: 20px;
        }

        .settings-container {
            grid-column: span 3;
            display: flex;
            flex-direction: column;
        }

        .settings-container .card {
            flex: 1;
            display: flex;
            flex-direction: column;
        }

        .theme-controls {
            display: grid;
            gap: 15px;
            flex: 1;
        }

        .theme-controls .btn {
            margin-top: auto;
        }

        .server-display {
            background: var(--bg-color);
            padding: 8px 12px;
            border-radius: 4px;
            border: 1px solid var(--border-color);
            display: flex;
            align-items: center;
            gap: 8px;
        }

        .edit-button {
            background: none;
            border: none;
            color: var(--text-color);
            cursor: pointer;
            padding: 8px 12px;
            border-radius: 4px;
            display: flex;
            align-items: center;
            gap: 6px;
        }

        .edit-button:hover {
            background: var(--border-color);
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

        .card, .chart-card {
            background: var(--card-bg);
            padding: 15px;
            border-radius: 12px;
            box-shadow: 0 4px 6px var(--shadow-color);
            transition: transform 0.2s ease, box-shadow 0.2s ease;
        }

        .card:hover, .chart-card:hover {
            transform: translateY(-2px);
            box-shadow: 0 6px 12px var(--shadow-color);
        }

        h1 {
            color: var(--primary-color);
            font-size: 1.5em;
            margin: 0;
        }

        h2 {
            color: var(--secondary-color);
            margin-bottom: 15px;
            font-size: 1.2em;
            display: flex;
            align-items: center;
            gap: 10px;
        }

        .info-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(120px, 1fr));
            gap: 10px;
        }

        .info-item {
            padding: 10px;
            border-radius: 6px;
            background: var(--bg-color);
            border: 1px solid var(--border-color);
            min-width: 0;
            overflow: hidden;
        }

        .info-label {
            font-size: 0.8em;
            color: var(--secondary-color);
            margin-bottom: 2px;
            display: flex;
            align-items: center;
            gap: 5px;
        }

        .info-value {
            font-size: 1em;
            font-weight: 500;
            white-space: nowrap;
            overflow: hidden;
            text-overflow: ellipsis;
            min-width: 0;
        }

        .chart-container {
            position: relative;
            height: 200px;
            width: 100%;
        }

        .glances-settings {
            display: flex;
            align-items: center;
            gap: 10px;
        }

        .server-info {
            display: flex;
            align-items: center;
            gap: 10px;
        }

        .btn {
            padding: 8px 16px;
            border: none;
            border-radius: 5px;
            color: white;
            cursor: pointer;
            transition: background-color 0.3s ease;
            display: flex;
            align-items: center;
            gap: 5px;
        }

        .btn-success {
            background-color: var(--success-color);
        }

        .btn-success:hover {
            background-color: #388E3C;
        }

        .btn-danger {
            background-color: var(--danger-color);
        }

        .btn-danger:hover {
            background-color: #d32f2f;
        }

        .theme-toggle {
            display: flex;
            align-items: center;
            gap: 10px;
        }

        .switch {
            position: relative;
            display: inline-block;
            width: 60px;
            height: 34px;
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
            background-color: #ccc;
            transition: .4s;
            border-radius: 34px;
        }

        .slider:before {
            position: absolute;
            content: "";
            height: 26px;
            width: 26px;
            left: 4px;
            bottom: 4px;
            background-color: white;
            transition: .4s;
            border-radius: 50%;
        }

        input:checked + .slider {
            background-color: var(--primary-color);
        }

        input:checked + .slider:before {
            transform: translateX(26px);
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

        @media (max-width: 1200px) {
            .charts-row {
                grid-template-columns: repeat(2, 1fr);
            }
            .stats-container {
                grid-column: span 12;
                grid-template-columns: repeat(2, 1fr);
            }
            .settings-container {
                grid-column: span 12;
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
                    <h2><i class="ri-temp-hot-line"></i> Temperature</h2>
                    <div class="chart-container">
                        <canvas id="tempChart"></canvas>
                    </div>
                </div>
                <div class="chart-card">
                    <h2><i class="ri-database-2-line"></i> Memory Usage</h2>
                    <div class="chart-container">
                        <canvas id="memoryChart"></canvas>
                    </div>
                </div>
                <div class="chart-card">
                    <h2><i class="ri-wifi-line"></i> WiFi Signal</h2>
                    <div class="chart-container">
                        <canvas id="wifiChart"></canvas>
                    </div>
                </div>
            </div>

            <div class="stats-container">
                <div class="card">
                    <h2><i class="ri-information-line"></i> System Info</h2>
                    <div class="info-grid">
                        <div class="info-item">
                            <div class="info-label"><i class="ri-cpu-line"></i> Model</div>
                            <div class="info-value" id="chipModel">---</div>
                        </div>
                        <div class="info-item">
                            <div class="info-label"><i class="ri-code-s-slash-line"></i> SDK Version</div>
                            <div class="info-value" id="sdkVersion">---</div>
                        </div>
                        <div class="info-item">
                            <div class="info-label"><i class="ri-cpu-line"></i> CPU Cores</div>
                            <div class="info-value" id="chipCores">---</div>
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
                    </div>
                </div>
                <div class="card">
                    <h2><i class="ri-database-2-line"></i> Memory Info</h2>
                    <div class="info-grid">
                        <div class="info-item">
                            <div class="info-label"><i class="ri-sd-card-line"></i> Flash Size</div>
                            <div class="info-value" id="flashSize">-</div>
                        </div>
                        <div class="info-item">
                            <div class="info-label"><i class="ri-database-2-line"></i> Free Heap</div>
                            <div class="info-value" id="freeHeap">-</div>
                        </div>
                        <div class="info-item">
                            <div class="info-label"><i class="ri-database-2-line"></i> Min Free Heap</div>
                            <div class="info-value" id="minFreeHeap">-</div>
                        </div>
                        <div class="info-item">
                            <div class="info-label"><i class="ri-database-2-line"></i> Max Alloc Heap</div>
                            <div class="info-value" id="maxAllocHeap">-</div>
                        </div>
                        <div class="info-item">
                            <div class="info-label"><i class="ri-database-2-line"></i> PSRAM Size</div>
                            <div class="info-value" id="psramSize">-</div>
                        </div>
                        <div class="info-item">
                            <div class="info-label"><i class="ri-database-2-line"></i> Free PSRAM</div>
                            <div class="info-value" id="freePsram">-</div>
                        </div>
                        <div class="info-item">
                            <div class="info-label"><i class="ri-file-code-line"></i> Sketch Size</div>
                            <div class="info-value" id="sketchSize">-</div>
                        </div>
                        <div class="info-item">
                            <div class="info-label"><i class="ri-file-code-line"></i> Free Sketch Space</div>
                            <div class="info-value" id="freeSketchSpace">-</div>
                        </div>
                    </div>
                </div>
                <div class="card">
                    <h2><i class="ri-wifi-line"></i> Network Info</h2>
                    <div class="info-grid">
                        <div class="info-item">
                            <div class="info-label"><i class="ri-wifi-line"></i> WiFi SSID</div>
                            <div class="info-value" id="wifiSSID">-</div>
                        </div>
                        <div class="info-item">
                            <div class="info-label"><i class="ri-signal-wifi-line"></i> Signal Strength</div>
                            <div class="info-value" id="wifiStrength">-</div>
                        </div>
                        <div class="info-item">
                            <div class="info-label"><i class="ri-ip-fill"></i> IP Address</div>
                            <div class="info-value" id="ipAddress">-</div>
                        </div>
                        <div class="info-item">
                            <div class="info-label"><i class="ri-mac-line"></i> MAC Address</div>
                            <div class="info-value" id="macAddress">-</div>
                        </div>
                        <div class="info-item">
                            <div class="info-label"><i class="ri-wifi-line"></i> WiFi Channel</div>
                            <div class="info-value" id="channel">-</div>
                        </div>
                        <div class="info-item">
                            <div class="info-label"><i class="ri-global-line"></i> DNS Server</div>
                            <div class="info-value" id="dnsIP">-</div>
                        </div>
                        <div class="info-item">
                            <div class="info-label"><i class="ri-router-line"></i> Gateway</div>
                            <div class="info-value" id="gatewayIP">-</div>
                        </div>
                        <div class="info-item">
                            <div class="info-label"><i class="ri-filter-line"></i> Subnet Mask</div>
                            <div class="info-value" id="subnetMask">-</div>
                        </div>
                    </div>
                </div>
            </div>
            
            <div class="settings-container">
                <div class="card">
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
            </div>
        </div>
    </div>

    <script>
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

        // Initialize charts
        const chartOptions = {
            responsive: true,
            maintainAspectRatio: false,
            scales: {
                y: {
                    beginAtZero: true
                }
            },
            animation: {
                duration: 0
            },
            plugins: {
                legend: {
                    display: false
                }
            }
        };

        const tempChart = new Chart(document.getElementById('tempChart').getContext('2d'), {
            type: 'line',
            data: {
                labels: [],
                datasets: [{
                    label: 'Temperature',
                    data: [],
                    borderColor: '#2196F3',
                    tension: 0.4
                }]
            },
            options: chartOptions
        });

        const memoryChart = new Chart(document.getElementById('memoryChart').getContext('2d'), {
            type: 'line',
            data: {
                labels: [],
                datasets: [{
                    label: 'Free Heap (KB)',
                    data: [],
                    borderColor: '#4CAF50',
                    tension: 0.4
                }]
            },
            options: chartOptions
        });

        const wifiChart = new Chart(document.getElementById('wifiChart').getContext('2d'), {
            type: 'line',
            data: {
                labels: [],
                datasets: [{
                    label: 'WiFi Signal (dBm)',
                    data: [],
                    borderColor: '#FFC107',
                    tension: 0.4
                }]
            },
            options: chartOptions
        });

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

        function updateCharts(data) {
            const timestamp = new Date().toLocaleTimeString();
            const maxDataPoints = 50;
            
            function updateChart(chart, newValue) {
                if (chart.data.labels.length > maxDataPoints) {
                    chart.data.labels.shift();
                    chart.data.datasets[0].data.shift();
                }
                chart.data.labels.push(timestamp);
                chart.data.datasets[0].data.push(newValue);
                chart.update();
            }

            updateChart(tempChart, data.temperature);
            updateChart(memoryChart, data.freeHeap);
            updateChart(wifiChart, data.wifiStrength);
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
            document.getElementById('chipModel').textContent = data.chipModel;
            document.getElementById('sdkVersion').textContent = data.sdkVersion;
            document.getElementById('chipCores').textContent = data.chipCores;
            document.getElementById('temperature').textContent = data.temperature + ' °C';
            document.getElementById('hallSensor').textContent = data.hallSensor;
            document.getElementById('uptime').textContent = formatUptime(data.uptime);
            
            document.getElementById('flashSize').textContent = data.flashSize + ' MB';
            document.getElementById('freeHeap').textContent = data.freeHeap + ' KB';
            document.getElementById('minFreeHeap').textContent = data.minFreeHeap + ' KB';
            document.getElementById('maxAllocHeap').textContent = data.maxAllocHeap + ' KB';
            document.getElementById('psramSize').textContent = data.psramSize + ' MB';
            document.getElementById('freePsram').textContent = data.freePsram + ' KB';
            document.getElementById('sketchSize').textContent = data.sketchSize + ' KB';
            document.getElementById('freeSketchSpace').textContent = data.freeSketchSpace + ' KB';
            
            document.getElementById('wifiSSID').textContent = data.wifiSSID;
            document.getElementById('wifiStrength').textContent = data.wifiStrength + ' dBm';
            document.getElementById('ipAddress').textContent = data.ipAddress;
            document.getElementById('macAddress').textContent = data.macAddress;
            document.getElementById('channel').textContent = data.channel;
            document.getElementById('dnsIP').textContent = data.dnsIP;
            document.getElementById('gatewayIP').textContent = data.gatewayIP;
            document.getElementById('subnetMask').textContent = data.subnetMask;
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

        // Modify the existing interval fetch to include server display update
        setInterval(() => {
            fetch('/settings')
                .then(response => response.json())
                .then(data => {
                    updateSystemInfo(data);
                    updateCharts(data);
                    updateColorPickers();
                    
                    // Only update Glances inputs if user is not currently editing
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

void handleRoot() {
    server.send(200, "text/html", INDEX_HTML);
}

void handleGetSettings() {
    StaticJsonDocument<1024> doc;
    
    // System stats
    doc["chipModel"] = ESP.getChipModel();
    doc["sdkVersion"] = ESP.getSdkVersion();
    doc["chipCores"] = ESP.getChipCores();
    doc["temperature"] = String((temperatureRead() - 32) / 1.8, 2);
    doc["hallSensor"] = hallRead();
    doc["uptime"] = millis() / 1000;
    
    // Memory info
    doc["flashSize"] = ESP.getFlashChipSize() / 1024;
    doc["freeHeap"] = ESP.getFreeHeap() / 1024;
    doc["minFreeHeap"] = ESP.getMinFreeHeap() / 1024;
    doc["maxAllocHeap"] = ESP.getMaxAllocHeap() / 1024;
    doc["psramSize"] = ESP.getPsramSize() / 1024;
    doc["freePsram"] = ESP.getFreePsram() / 1024;
    doc["sketchSize"] = ESP.getSketchSize() / 1024;
    doc["freeSketchSpace"] = ESP.getFreeSketchSpace() / 1024;
    
    // Network info
    doc["wifiSSID"] = WiFi.SSID();
    doc["wifiStrength"] = WiFi.RSSI();
    doc["ipAddress"] = WiFi.localIP().toString();
    doc["macAddress"] = WiFi.macAddress();
    doc["channel"] = WiFi.channel();
    doc["dnsIP"] = WiFi.dnsIP().toString();
    doc["gatewayIP"] = WiFi.gatewayIP().toString();
    doc["subnetMask"] = WiFi.subnetMask().toString();
    
    // Theme Colors
    const ThemeColors& theme = SettingsManager::getCurrentTheme();
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

void handleUpdateSettings() {
    String json = server.arg("plain");
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, json);
    
    if (!error) {
        if (doc.containsKey("darkMode")) {
            SettingsManager::setDarkMode(doc["darkMode"].as<bool>());
        }
        if (doc.containsKey("bg_color")) {
            uint32_t color = doc["bg_color"].as<uint32_t>();
            Serial.printf("Updating bg_color to: %06X\n", color);
            SettingsManager::updateThemeColor("bg_color", color);
        }
        if (doc.containsKey("text_color")) {
            SettingsManager::updateThemeColor("text_color", doc["text_color"].as<uint32_t>());
        }
        if (doc.containsKey("cpu_color")) {
            SettingsManager::updateThemeColor("cpu_color", doc["cpu_color"].as<uint32_t>());
        }
        if (doc.containsKey("ram_color")) {
            SettingsManager::updateThemeColor("ram_color", doc["ram_color"].as<uint32_t>());
        }
        if (doc.containsKey("border_color")) {
            SettingsManager::updateThemeColor("border_color", doc["border_color"].as<uint32_t>());
        }
        if (doc.containsKey("card_bg_color")) {
            SettingsManager::updateThemeColor("card_bg_color", doc["card_bg_color"].as<uint32_t>());
        }
        // Add these conditions to the existing if block
        if (doc.containsKey("glances_host")) {
            SettingsManager::setGlancesHost(doc["glances_host"].as<String>());
        }
        if (doc.containsKey("glances_port")) {
            SettingsManager::setGlancesPort(doc["glances_port"].as<uint16_t>());
        }
        server.send(200, "application/json", "{\"status\":\"success\"}");
    } else {
        server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid JSON\"}");
    }
}

void handleRestart() {
    server.send(200, "application/json", "{\"status\":\"success\"}");
    delay(500);
    ESP.restart();
}

void handleResetTheme() {
    // Reset to original theme colors
    if (SettingsManager::getDarkMode()) {
        const_cast<ThemeColors&>(SettingsManager::getCurrentTheme()) = dark_theme;
    } else {
        const_cast<ThemeColors&>(SettingsManager::getCurrentTheme()) = light_theme;
    }
    
    // Clear saved colors from preferences
    SettingsManager::clearSavedColors();
    
    // Update the display
    if (SettingsManager::themeCallback) {
        SettingsManager::themeCallback(SettingsManager::getDarkMode());
    }
    
    server.send(200, "application/json", "{\"status\":\"success\"}");
}

// Add these new REST API endpoints for Home Assistant
void handleHaStatus() {
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

void handleHaCommand() {
    if (server.method() != HTTP_POST) {
        server.send(405, "application/json", "{\"error\":\"Method not allowed\"}");
        return;
    }

    String json = server.arg("plain");
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, json);

    if (error) {
        server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        return;
    }

    bool success = false;
    String message = "Unknown command";

    if (doc.containsKey("dark_mode")) {
        SettingsManager::setDarkMode(doc["dark_mode"].as<bool>());
        success = true;
        message = "Dark mode updated";
    }
    
    if (doc.containsKey("restart")) {
        if (doc["restart"].as<bool>()) {
            success = true;
            message = "Restarting device";
            // Schedule restart after sending response
            server.send(200, "application/json", "{\"success\":true,\"message\":\"Restarting device\"}");
            delay(500);
            ESP.restart();
            return;
        }
    }

    if (doc.containsKey("reset_theme")) {
        if (doc["reset_theme"].as<bool>()) {
            // Reset theme to defaults
            if (SettingsManager::getDarkMode()) {
                const_cast<ThemeColors&>(SettingsManager::getCurrentTheme()) = dark_theme;
            } else {
                const_cast<ThemeColors&>(SettingsManager::getCurrentTheme()) = light_theme;
            }
            SettingsManager::clearSavedColors();
            if (SettingsManager::themeCallback) {
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

void setupWebServer() {
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

void handleWebServer() {
    server.handleClient();
}