#include "settings_manager.h"
#include "config.h"
#include <lvgl.h>
#include <string.h>

Preferences SettingsManager::preferences;
bool SettingsManager::darkMode = true;
SettingsManager::ThemeCallback SettingsManager::themeCallback = nullptr;


static ThemeColors mutable_dark_theme = dark_theme;
static ThemeColors mutable_light_theme = light_theme;

String SettingsManager::glancesHost;
uint16_t SettingsManager::glancesPort;

void SettingsManager::begin() {
    preferences.begin("settings", false);
    darkMode = preferences.getBool("darkMode", true);

    // Load Glances settings and update global variables
    glancesHost = preferences.getString("glances_host", "");
    glancesPort = preferences.getUInt("glances_port", 61208);

    // Update the global variables from config.cpp
    glances_host = glancesHost;
    glances_port = glancesPort;

    // Initialize mutable themes with defaults first
    mutable_dark_theme = dark_theme;
    mutable_light_theme = light_theme;

    // Then load any saved colors if they exist
    if (preferences.isKey("bg_color")) {
        uint32_t color = preferences.getUInt("bg_color", 0);
        mutable_dark_theme.bg_color = lv_color_hex(color);
    }
    if (preferences.isKey("text_color")) {
        uint32_t color = preferences.getUInt("text_color", 0);
        mutable_dark_theme.text_color = lv_color_hex(color);
    }
    if (preferences.isKey("cpu_color")) {
        uint32_t color = preferences.getUInt("cpu_color", 0);
        mutable_dark_theme.cpu_color = lv_color_hex(color);
    }
    if (preferences.isKey("ram_color")) {
        uint32_t color = preferences.getUInt("ram_color", 0);
        mutable_dark_theme.ram_color = lv_color_hex(color);
    }

    if (themeCallback) {
        themeCallback(darkMode);
    }
}

bool SettingsManager::getDarkMode() {
    return darkMode;
}

void SettingsManager::setDarkMode(bool enabled) {
    darkMode = enabled;
    saveSettings();
    if (themeCallback) {
        themeCallback(enabled);
    }
}

void SettingsManager::saveSettings() {
    preferences.putBool("darkMode", darkMode);
}

void SettingsManager::updateThemeColor(const char* colorName, uint32_t color) {
    ThemeColors& theme = darkMode ? mutable_dark_theme : mutable_light_theme;
    
    if (strcmp(colorName, "bg_color") == 0) {
        theme.bg_color = lv_color_hex(color);
        preferences.putUInt("bg_color", color);
    }
    else if (strcmp(colorName, "text_color") == 0) {
        theme.text_color = lv_color_hex(color);
        preferences.putUInt("text_color", color);
    }
    else if (strcmp(colorName, "cpu_color") == 0) {
        theme.cpu_color = lv_color_hex(color);
        preferences.putUInt("cpu_color", color);
    }
    else if (strcmp(colorName, "ram_color") == 0) {
        theme.ram_color = lv_color_hex(color);
        preferences.putUInt("ram_color", color);
    }

    // Update the display
    if (themeCallback) {
        themeCallback(darkMode);
    }
}

const ThemeColors& SettingsManager::getCurrentTheme() {
    return darkMode ? mutable_dark_theme : mutable_light_theme;
}

void SettingsManager::clearSavedColors() {
    preferences.remove("bg_color");
    preferences.remove("text_color");
    preferences.remove("cpu_color");
    preferences.remove("ram_color");
}

const String& SettingsManager::getGlancesHost() {
    return glancesHost;
}

uint16_t SettingsManager::getGlancesPort() {
    return glancesPort;
}

void SettingsManager::setGlancesHost(const String& host) {
    glancesHost = host;
    preferences.putString("glances_host", host);
    glances_host = host;
}

void SettingsManager::setGlancesPort(uint16_t port) {
    glancesPort = port;
    preferences.putUInt("glances_port", port);
    glances_port = port;
}