#ifndef CONFIG_H
#define CONFIG_H

#include <lvgl.h>
#include <WString.h>

// WiFi Credentials
extern const char* const ssid;
extern const char* const password;

// Glances API Config
extern String glances_host;
extern uint16_t glances_port;
#define GLANCES_UPDATE_INTERVAL 2000

// Screen resolution
extern const uint16_t screenWidth;
extern const uint16_t screenHeight;

// Theme structure and definitions
struct ThemeColors
{
    lv_color_t bg_color;      // Background color
    lv_color_t text_color;    // Foreground/text color
    lv_color_t cpu_color;     // Primary (used for CPU arc)
    lv_color_t ram_color;     // Secondary (used for RAM arc)
};

extern const ThemeColors light_theme;
extern const ThemeColors dark_theme;

#define DARK_MODE true

#endif