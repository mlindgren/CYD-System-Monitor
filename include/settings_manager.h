#pragma once

#include <Preferences.h>
#include <functional>
#include "config.h"

class SettingsManager {
public:
    using ThemeCallback = std::function<void(bool)>;

    static void begin();
    static bool getDarkMode();
    static void setDarkMode(bool isDark);
    static void saveSettings();
    static void updateThemeColor(const char* colorName, uint32_t color);
    static const ThemeColors& getCurrentTheme();
    static void setThemeChangeCallback(ThemeCallback callback) {
        themeCallback = callback;
    }
    static void clearSavedColors();
    static const String& getGlancesHost();
    static uint16_t getGlancesPort();
    static void setGlancesHost(const String& host);
    static void setGlancesPort(uint16_t port);

    static ThemeCallback themeCallback;

private:
    static Preferences preferences;
    static bool darkMode;
    static void loadSettings();
    static String glancesHost;
    static uint16_t glancesPort;
};