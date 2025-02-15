# ESP32 System Monitor

A sleek system monitoring display powered by ESP32 that shows real-time system metrics from a Glances server. Features a customizable UI with dark/light theme support using LVGL graphics library.

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

- Home Assistant integration:
  - REST API endpoints
  - Device status monitoring & control
  - Remote theme control

## Requirements

- ESP32 development board
- TFT display compatible with TFT_eSPI library

I'm using this cheap yellow display with ESP32 built in: [aliexpress](https://s.click.aliexpress.com/e/_olrdG2w)
The settings in this project are for this display.

- PlatformIO
- Glances server running on the target system
- Arduino framework
- Required libraries (automatically managed by PlatformIO):
  - LVGL
  - TFT_eSPI
  - ArduinoJson
  - ESP32WebServer
  - Preferences

## Setup

1. Clone this repository
2. Open the project in PlatformIO

   - Edit the `platformio.ini` file to set the correct path to your Arduino libraries

3. Configure your TFT display settings:
   - Modify TFT_eSPI settings according to your display's configuration
   - Adjust screen resolution in config.h if needed:

   ```cpp

   extern const uint16_t screenWidth = 240;
   extern const uint16_t screenHeight = 320;

   ```

4. Configure your network settings in config.cpp:

   ```cpp

   const char*const ssid = "YOUR_WIFI_SSID";
   const char* const password = "YOUR_WIFI_PASSWORD";

   ```

5. Build and upload the project using PlatformIO

6. Set up your Glances server configuration in the web interface:

   - Access the web interface at the device's IP address
   - Configure the Glances server IP address and port
   - Choose theme colors if you want to change them
   - Save the configuration

### Web Interface

The device hosts a web interface for configuration, accessible at its IP address. Features include:

- Theme switching (dark/light)
- Color customization
- System statistics
- Device restart
- Theme reset

### Home Assistant Integration

The device exposes REST API endpoints for Home Assistant integration:

- GET `/api/status` - Device status and metrics
- POST `/api/command` - Control endpoints for theme switching and device restart

## API Endpoints

### Web Interface Endpoints

- GET `/settings` - Current device and theme settings
- POST `/settings` - Update device settings
- POST `/restart` - Restart device
- POST `/resetTheme` - Reset theme to defaults

### Home Assistant Endpoints

- GET `/api/status` - Device status and metrics
- POST `/api/command` - Control endpoints for:
  - Theme switching
  - Device restart
  - Theme reset

## Contributing

Feel free to submit issues, fork the repository, and create pull requests for any improvements.

## License
