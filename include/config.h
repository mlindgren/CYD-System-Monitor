#ifndef CONFIG_H
#define CONFIG_H

#include <lvgl.h>
#include <WString.h>

extern const char *const ssid;
extern const char *const password;

extern String glances_host;
extern uint16_t glances_port;
#define GLANCES_UPDATE_INTERVAL 2000

// Debug configuration
extern bool debug_mode;
#define DEBUG_PRINT(x) if(debug_mode) { Serial.print(x); }
#define DEBUG_PRINTLN(x) if(debug_mode) { Serial.println(x); }
#define DEBUG_PRINTF(format, ...) if(debug_mode) { Serial.printf(format, __VA_ARGS__); }

extern const uint16_t screenWidth;
extern const uint16_t screenHeight;
struct ThemeColors
{
    lv_color_t bg_color;
    lv_color_t card_bg_color;
    lv_color_t text_color;
    lv_color_t cpu_color;
    lv_color_t ram_color;
    lv_color_t border_color;
};

extern const ThemeColors light_theme;
extern const ThemeColors dark_theme;

#define DARK_MODE true

#endif