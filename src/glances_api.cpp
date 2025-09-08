#include "glances_api.h"
#include "gui.h"
#include "config.h"
#include <HTTPClient.h>
#include <WiFi.h>

bool GlancesAPI::fetchData(const char *endpoint, StaticJsonDocument<4096> &doc)
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("WiFi not connected for Glances API");
        return false;
    }

    if (glances_host.length() == 0) {
        Serial.println("Glances host not configured");
        return false;
    }

    HTTPClient http;
    String url = "http://" + glances_host + ":" + String(glances_port) + endpoint;
    DEBUG_PRINTF("Fetching: %s\n", url.c_str());
    
    http.begin(url);
    http.setTimeout(5000); // 5 second timeout
    int httpCode = http.GET();

    if (httpCode != HTTP_CODE_OK)
    {
        Serial.printf("HTTP error %d for endpoint %s\n", httpCode, endpoint);
        http.end();
        return false;
    }

    String payload = http.getString();
    http.end();

    DeserializationError error = deserializeJson(doc, payload);
    if (error) {
        Serial.printf("JSON parse error for %s: %s\n", endpoint, error.c_str());
        return false;
    }
    
    return true;
}

void GlancesAPI::updateCPUData(StaticJsonDocument<4096> &doc)
{
    if (!fetchData("/api/4/cpu", doc)) {
        Serial.printf("Failed to fetch CPU data");
        return;
    }

    float cpuPercent = doc["total"].as<float>();
    int cpuCount = doc["cpucore"].as<int>();
    
    DEBUG_PRINTF("CPU: %.1f%%, Cores: %d\n", cpuPercent, cpuCount);

    if (cpu_arc_obj.arc && cpu_arc_obj.label)
    {
        lv_obj_t **labels = (lv_obj_t **)lv_obj_get_user_data(cpu_arc_obj.arc);
        if (labels)
        {
            char buf[32];

            lv_label_set_text(labels[0], "CPU");
            snprintf(buf, sizeof(buf), "%d cores", cpuCount);
            lv_label_set_text(labels[1], buf);
            snprintf(buf, sizeof(buf), "%d%%", (int)cpuPercent);
            lv_label_set_text(labels[2], buf);
            lv_obj_set_style_text_font(labels[1], &lv_font_montserrat_10, 0);
            lv_obj_set_style_text_font(labels[2], &lv_font_montserrat_16, 0);
            lv_obj_set_style_text_color(labels[1], lv_color_hex(0x808080), 0);
            lv_obj_set_style_text_color(labels[2], lv_color_white(), 0);
            
            DEBUG_PRINTLN("Updated CPU arc labels");
        } else {
            DEBUG_PRINTLN("CPU arc labels not found");
        }
        set_arc_value_animated(cpu_arc_obj.arc, cpuPercent, 500);
        DEBUG_PRINTF("Set CPU arc to %d%%\n", (int)cpuPercent);
    } else {
        DEBUG_PRINTLN("CPU arc object not initialized");
    }
}

void GlancesAPI::updateMemoryData(StaticJsonDocument<4096> &doc)
{
    if (!fetchData("/api/4/mem", doc))
        return;

    float memPercent = doc["percent"].as<float>();
    float totalRam = doc["total"].as<float>() / (1024.0 * 1024.0 * 1024.0);

    if (ram_arc_obj.arc && ram_arc_obj.label)
    {
        lv_obj_t **labels = (lv_obj_t **)lv_obj_get_user_data(ram_arc_obj.arc);
        if (labels)
        {
            char buf[32];

            lv_label_set_text(labels[0], "RAM");
            snprintf(buf, sizeof(buf), "%d%%", (int)memPercent);
            lv_label_set_text(labels[1], buf);
            snprintf(buf, sizeof(buf), "/ %.1f GB", totalRam);
            lv_label_set_text(labels[2], buf);
        }
        set_arc_value_animated(ram_arc_obj.arc, memPercent);
    }
}

void updateGlancesData()
{
    static unsigned long lastGlancesUpdate = 0;
    static bool first_run = true;
    
    if (millis() - lastGlancesUpdate < GLANCES_UPDATE_INTERVAL)
    {
        return;
    }
    
    if (first_run) {
        Serial.println("Starting Glances data updates...");
        Serial.printf("Glances Host: %s\n", glances_host.c_str());
        Serial.printf("Glances Port: %d\n", glances_port);
        Serial.printf("Debug Mode: %s\n", debug_mode ? "Enabled" : "Disabled");
        first_run = false;
    }
    
    static StaticJsonDocument<4096> doc;
    
    DEBUG_PRINTLN("Updating CPU data...");
    GlancesAPI::updateCPUData(doc);
    
    DEBUG_PRINTLN("Updating Memory data...");
    GlancesAPI::updateMemoryData(doc);

    if (GlancesAPI::fetchData("/api/4/sensors", doc))
    {
        for (JsonVariant sensor : doc.as<JsonArray>())
        {
            if (strcmp(sensor["label"], "Package id 0") == 0)
            {
                int temp = (int)sensor["value"].as<float>();
                char buf[32];
                snprintf(buf, sizeof(buf), LV_SYMBOL_WARNING " Temp: %d°C", temp);
                update_compact_label(temp_label, buf);
                break;
            }
        }
    }

    DEBUG_PRINTLN("Updating disk data...");
    if (GlancesAPI::fetchData("/api/4/fs", doc))
    {
        unsigned long long totalSize = 0;
        unsigned long long usedSize = 0;
        int driveCount = 0;

        DEBUG_PRINTLN("Processing filesystem data:");
        for (JsonVariant fs : doc.as<JsonArray>())
        {
            const char *mnt_point = fs["mnt_point"].as<const char *>();
            const char *fs_type = fs["fs_type"].as<const char *>();
            const char *options = fs["options"].as<const char *>();
            
            DEBUG_PRINTF("  Drive: %s, Type: %s, Options: %s\n", mnt_point, fs_type, options);
            
            // For Windows: Include fixed drives (C:\, D:\, etc.), exclude removable and CD-ROM
            // For Linux: Include specific mount points or all non-system mounts
            bool includeInArray = false;
            
            if (strstr(options, "fixed") != nullptr && strstr(options, "rw") != nullptr) {
                // Windows fixed drives (C:\, D:\, etc.)
                includeInArray = true;
            } else if (strncmp(mnt_point, "/rootfs/mnt/disk", 15) == 0) {
                // Linux unRAID array disks
                includeInArray = true;
            } else if (mnt_point[0] == '/' && 
                      strcmp(mnt_point, "/") != 0 && 
                      strstr(mnt_point, "/boot") == nullptr &&
                      strstr(mnt_point, "/snap") == nullptr &&
                      strstr(mnt_point, "/sys") == nullptr &&
                      strstr(mnt_point, "/proc") == nullptr &&
                      strstr(mnt_point, "/dev") == nullptr) {
                // Linux regular mount points (excluding system mounts)
                includeInArray = true;
            }
            
            if (includeInArray) {
                totalSize += fs["size"].as<unsigned long long>();
                usedSize += fs["used"].as<unsigned long long>();
                driveCount++;
                DEBUG_PRINTF("    Added to array: %s\n", mnt_point);
            }
        }

        if (totalSize > 0)
        {
            float usagePercent = (usedSize * 100.0) / totalSize;
            char buf[32];
            snprintf(buf, sizeof(buf), LV_SYMBOL_DRIVE " Drives: %.1f%%", usagePercent);
            update_compact_label(disk_label, buf);
            DEBUG_PRINTF("Updated disk array: %.1f%% (%d drives)\n", usagePercent, driveCount);
        } else {
            DEBUG_PRINTLN("No drives found for array display");
        }
    }

    DEBUG_PRINTLN("Updating cache data...");
    if (GlancesAPI::fetchData("/api/4/fs", doc))
    {
        bool cacheFound = false;
        for (JsonVariant fs : doc.as<JsonArray>())
        {
            const char *mnt_point = fs["mnt_point"].as<const char *>();
            
            // Check for various cache mount points
            if (strcmp(mnt_point, "/rootfs/mnt/cache") == 0 ||  // unRAID cache
                strcmp(mnt_point, "/cache") == 0 ||             // Generic cache
                strcmp(mnt_point, "/var/cache") == 0 ||         // System cache
                strstr(mnt_point, "cache") != nullptr) {        // Any mount containing "cache"
                
                float usage = fs["percent"].as<float>();
                char buf[32];
                snprintf(buf, sizeof(buf), LV_SYMBOL_SAVE " Cache: %.1f%%", usage);
                update_compact_label(cache_label, buf);
                DEBUG_PRINTF("Updated cache: %.1f%% (%s)\n", usage, mnt_point);
                cacheFound = true;
                break;
            }
        }
        
        if (!cacheFound) {
            DEBUG_PRINTLN("No cache or suitable drive found");
        }
    }

    if (GlancesAPI::fetchData("/api/4/uptime", doc))
    {
        String payload = doc.as<String>();
        payload.replace("\"", "");
        char buf[32];
        snprintf(buf, sizeof(buf), LV_SYMBOL_POWER "  %s", payload.c_str());
        update_compact_label(uptime_label, buf);
    }

    if (GlancesAPI::fetchData("/api/4/network", doc))
    {
        for (JsonVariant interface : doc.as<JsonArray>())
        {
            const char *interface_name = interface["interface_name"].as<const char *>();

            if (strcmp(interface_name, "eth0") == 0)
            {
                float recv_rate = interface["bytes_recv_rate_per_sec"].as<float>();
                float sent_rate = interface["bytes_sent_rate_per_sec"].as<float>();

                char down_str[16], up_str[16];
                auto formatSpeed = [](float bytes_per_sec, char *buffer)
                {
                    if (bytes_per_sec > 1024 * 1024)
                        sprintf(buffer, "%.1fM", bytes_per_sec / (1024.0 * 1024.0));
                    else if (bytes_per_sec > 1024)
                        sprintf(buffer, "%.1fK", bytes_per_sec / 1024.0);
                    else
                        sprintf(buffer, "%.0fB", bytes_per_sec);
                };

                formatSpeed(recv_rate, down_str);
                formatSpeed(sent_rate, up_str);

                char buf[64];
                snprintf(buf, sizeof(buf), LV_SYMBOL_DOWNLOAD " %s    " LV_SYMBOL_UPLOAD " %s", down_str, up_str);
                update_compact_label(network_label, buf);
                break;
            }
        }
    }

    if (GlancesAPI::fetchData("/api/4/load", doc))
    {
        float load1 = doc["min1"].as<float>();
        char buf[32];
        snprintf(buf, sizeof(buf), LV_SYMBOL_CHARGE " Load: %.1f", load1);
        update_compact_label(load_label, buf);
    }

    lastGlancesUpdate = millis();
}