# CYD System Monitor (ESP32 "Cheap Yellow Display")

A sleek system monitoring display powered by ESP32 that shows real-time system metrics from a Glances server. Features a customizable UI with dark/light theme support using LVGL graphics library and power-saving display controls.

Forked from [iamlite/CYD-System-Monitor](https://github.com/iamlite/CYD-System-Monitor) to add compatibility for the ESP32-2432S028R and Windows hosts.

![Unraid](Images/device.jpeg)

## Features

- Real-time monitoring of:
  - CPU usage with core count and load average
  - RAM utilization with total capacity
  - Disk array usage percentage
  - Cache usage percentage
  - System temperature with color-coded warnings
  - Network traffic (upload/download) with auto-scaling units (B/KB/MB)
  - System load
  - Uptime

- Web interface for configuration:
  - Real-time theme customization
  - System statistics dashboard
  - Glances server configuration
  - Device control and monitoring
  - Display power management

![Web UI](Images/web.png)

- Home Assistant integration:
  - REST API endpoints
  - Device status monitoring & control
  - Remote theme control

## Hardware Requirements

Tested on **ESP32-2432S028R** ("Cheap Yellow Display")
  - ESP32-D0WD-V3 (revision v3.1) 
  - 2.8" ILI9341 TFT Display (320x240)
  - Integrated touch controller
  - Built-in USB-to-serial converter

Alternatively, it should work with any ESP32 development board a TFT display compatible with 
the lvgl and TFT_eSPI libraries

## Setup

### 1. Install PlatformIO
Visit [https://platformio.org/](https://platformio.org/) and install PlatformIO IDE or the
PlatformIO Core CLI.

### 2. Install USB Drivers (if needed)
For ESP32-2432S028R on Windows:
- Download and install CH340 drivers from the manufacturer
- Verify the device appears in Device Manager after connection (should be under "Ports (COM & LPT)"
  as USB-SERIAL CH340 or something similar)

You can also test connectivity with `esptool`:

```bash
$ pip install esptool # Install first if necessary
$ esptool read-mac
esptool v5.0.2
Connected to ESP32 on COM3:
Chip type:          ESP32-D0WD-V3 (revision v3.1)
Features:           Wi-Fi, BT, Dual Core + LP Core, 240MHz, Vref calibration in eFuse, Coding Scheme None
...
```

### 3. Clone the repo

```bash
$ git clone https://github.com/your-username/CYD-System-Monitor.git
$ cd CYD-System-Monitor
```

### 4. Configure PlatformIO

Rename `platformio.example.ini` to `platformio.ini` and update the library path:

```ini
# Default Arduino library locations:
# Windows: C:\Users\<username>\Documents\Arduino\libraries
# Linux: ~/Arduino/libraries
# macOS: ~/Documents/Arduino/libraries

lib_extra_dirs = /path/to/your/Arduino/libraries
```

### 5. Configure Network Credentials

Rename `credentials.example.h` to `credentials.h` and update:

```cpp
const char* const WIFI_SSID = "your_wifi_network";
const char* const WIFI_PASSWORD = "your_wifi_password";
```

### 6. Build and Upload

```bash
# Build the project
pio run

# Upload firmware
pio run -t upload

# Upload filesystem (web files)
pio run -t uploadfs

# Monitor serial output
pio device monitor
```

### 7. Configure Glances Server

#### Option A: Use Web Interface
1. Connect to the CYD device's IP address in your browser (you can see the IP in the serial output when the device boots)
2. Configure Glances server host and port in the settings

#### Option B: Use CURL
```bash
curl -X POST http://[ESP32_IP]/settings \
  -H "Content-Type: application/json" \
  -d '{"glances_host": "192.168.1.50", "glances_port": 61208}'
```

## Troubleshooting

### Display Not Working
1. Check TFT_eSPI configuration matches your hardware
2. Verify pin connections in `User_Setup.h`
3. Enable debug mode to see initialization messages

### No Glances Data
1. Verify Glances server is running and accessible
2. Check network connectivity 
3. Enable debug mode to see API call details

Some Glances modules can be very slow on Windows, which can cause the ESP32 application to become
unresponsive while waiting on HTTP calls. You may need to disable slow modules in glances.conf,
particularly `processcount` and `sensors`. See https://github.com/nicolargo/glances/issues/3046

### USB Connection Issues
1. Install CH340 drivers on Windows
2. Check cable supports data transfer (not just power)
3. Try different USB ports
4. Verify device appears in Device Manager/system logs

## Debug Mode

To print extra diagnostic messages, you can enable debug mode either by setting
`bool debug_mode = true;` in config.cpp, or at runtime with `curl`:

```bash
$ curl -X POST http://[ESP32_IP]/settings \
  -H "Content-Type: application/json" \
  -d '{"debug_mode": true}'
```

## Configuration Files

### Core Configuration
- `credentials.h` - WiFi network settings
- `platformio.ini` - Build configuration and library paths
- `include/config.h` - Display resolution and debug settings

### Hardware Configuration  
- `include/User_Setup.h` - TFT_eSPI pin mappings for CYD
- Pin configuration automatically applied via build flags

## API Endpoints

### Web Interface Endpoints

- GET `/settings` - Returns:
  - Current device settings and theme colors
  - System metrics (CPU, memory, temperature)
  - Network information
  - Device information (chip model, SDK version, etc.)
  - Hardware statistics (heap, PSRAM, flash)

- POST `/settings` - Update device settings:
  - Theme colors
  - Dark/light mode
  - Glances server configuration

- POST `/restart` - Restart device
- POST `/resetTheme` - Reset theme to defaults
- POST `/displaySleep` - Control display power state:

  ```json
  {
    "sleep": true|false
  }
  ```

### Home Assistant Endpoints

- GET `/api/status` - Returns:
  - Temperature
  - Free heap memory
  - WiFi signal strength
  - Uptime
  - Dark mode state
  - Display state

- POST `/api/command` - Control endpoints for:
  - Theme switching (`dark_mode`: true|false)
  - Display power (`display`: true|false)
  - Device restart (`restart`: true)
  - Theme reset (`reset_theme`: true)

## Contributing

Feel free to submit issues, fork the repository, and create pull requests for any improvements.
