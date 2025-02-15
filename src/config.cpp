#include "config.h"
#include "credentials.h"
#include <Arduino.h>

// Screen resolution
const uint16_t screenWidth = 240;
const uint16_t screenHeight = 320;

// Glances settings
String glances_host;
uint16_t glances_port;

// Theme definitions
const ThemeColors light_theme = {
    .bg_color = lv_color_hex(0xF0F0F0),
    .text_color = lv_color_hex(0x000000),
    .cpu_color = lv_color_hex(0x07FFF7),
    .ram_color = lv_color_hex(0xAE17FF)
};

const ThemeColors dark_theme = {
    .bg_color = lv_color_hex(0x121212),
    .text_color = lv_color_hex(0xFFFFFF),
    .cpu_color = lv_color_hex(0x07FFF7),
    .ram_color = lv_color_hex(0xAE17FF)
};