#include "config.h"
#include "credentials.h"
#include <Arduino.h>

// Screen resolution
const uint16_t screenWidth = 320;
const uint16_t screenHeight = 240;

// Debug mode - set to true for verbose logging, false for quiet operation
bool debug_mode = false;

// Glances settings
String glances_host;
uint16_t glances_port;

// Theme definitions
const ThemeColors light_theme = {
    .bg_color = lv_color_hex(0xF0F0F0),
    .card_bg_color = lv_color_hex(0xFFFFFF),
    .text_color = lv_color_hex(0x000000),
    .cpu_color = lv_color_hex(0x07FFF7),
    .ram_color = lv_color_hex(0xAE17FF),
    .border_color = lv_color_hex(0x404040)
};

const ThemeColors dark_theme = {
    .bg_color = lv_color_hex(0x121212),
    .card_bg_color = lv_color_hex(0x1E1E1E),
    .text_color = lv_color_hex(0xFFFFFF),
    .cpu_color = lv_color_hex(0x07FFF7),
    .ram_color = lv_color_hex(0xAE17FF),
    .border_color = lv_color_hex(0x404040)
};