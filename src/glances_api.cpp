#include "glances_api.h"
#include "gui.h"
#include "config.h"
#include <HTTPClient.h>
#include <WiFi.h>

bool GlancesAPI::fetchData(const char *endpoint, StaticJsonDocument<4096> &doc)
{
    if (WiFi.status() != WL_CONNECTED)
    {
        return false;
    }

    HTTPClient http;
    String url = "http://" + glances_host + ":" + String(glances_port) + endpoint;
    http.begin(url);
    int httpCode = http.GET();

    if (httpCode != HTTP_CODE_OK)
    {
        http.end();
        return false;
    }

    String payload = http.getString();
    http.end();

    DeserializationError error = deserializeJson(doc, payload);
    return !error;
}

void GlancesAPI::updateCPUData(StaticJsonDocument<4096> &doc)
{
    if (!fetchData("/api/4/cpu", doc))
        return;

    float cpuPercent = doc["total"].as<float>();
    int cpuCount = doc["cpucore"].as<int>();

    if (cpu_arc_obj.arc && cpu_arc_obj.label) {
        lv_obj_t **labels = (lv_obj_t **)lv_obj_get_user_data(cpu_arc_obj.arc);
        if (labels) {
            char buf[32];

            // Update title
            lv_label_set_text(labels[0], "CPU");

            // Update cores (now in the middle)
            snprintf(buf, sizeof(buf), "%d cores", cpuCount);
            lv_label_set_text(labels[1], buf);

            // Update percentage (now at the bottom)
            snprintf(buf, sizeof(buf), "%d%%", (int)cpuPercent);
            lv_label_set_text(labels[2], buf);

            // Adjust the font sizes and colors
            lv_obj_set_style_text_font(labels[1], &lv_font_montserrat_10, 0);  // cores small
            lv_obj_set_style_text_font(labels[2], &lv_font_montserrat_16, 0);  // percentage big
            lv_obj_set_style_text_color(labels[1], lv_color_hex(0x808080), 0); // cores gray
            lv_obj_set_style_text_color(labels[2], lv_color_white(), 0);       // percentage white
        }
        set_arc_value_animated(cpu_arc_obj.arc, cpuPercent);
    }
}

void GlancesAPI::updateMemoryData(StaticJsonDocument<4096> &doc)
{
    if (!fetchData("/api/4/mem", doc))
        return;

    float memPercent = doc["percent"].as<float>();
    float totalRam = doc["total"].as<float>() / (1024.0 * 1024.0 * 1024.0);

    if (ram_arc_obj.arc && ram_arc_obj.label) {
        lv_obj_t **labels = (lv_obj_t **)lv_obj_get_user_data(ram_arc_obj.arc);
        if (labels) {
            char buf[32];

            // Update title
            lv_label_set_text(labels[0], "RAM");

            // Update percentage
            snprintf(buf, sizeof(buf), "%d%%", (int)memPercent);
            lv_label_set_text(labels[1], buf);

            // Update total RAM
            snprintf(buf, sizeof(buf), "/ %.1f GB", totalRam);
            lv_label_set_text(labels[2], buf);
        }
        set_arc_value_animated(ram_arc_obj.arc, memPercent);
    }
}

void updateGlancesData()
{
    static unsigned long lastGlancesUpdate = 0;
    if (millis() - lastGlancesUpdate < GLANCES_UPDATE_INTERVAL)
    {
        return;
    }

    static StaticJsonDocument<4096> doc;

    // Update CPU and Memory
    GlancesAPI::updateCPUData(doc);
    GlancesAPI::updateMemoryData(doc);

    // Temperature
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

    // Disk usage
    if (GlancesAPI::fetchData("/api/4/fs", doc))
    {
        unsigned long long totalSize = 0;
        unsigned long long usedSize = 0;

        for (JsonVariant fs : doc.as<JsonArray>())
        {
            const char *mnt_point = fs["mnt_point"].as<const char *>();
            if (strncmp(mnt_point, "/rootfs/mnt/disk", 15) == 0)
            {
                totalSize += fs["size"].as<unsigned long long>();
                usedSize += fs["used"].as<unsigned long long>();
            }
        }

        if (totalSize > 0)
        {
            float usagePercent = (usedSize * 100.0) / totalSize;
            char buf[32];
            snprintf(buf, sizeof(buf), LV_SYMBOL_DRIVE " Array: %.1f%%", usagePercent);
            update_compact_label(disk_label, buf);
        }
    }

    // Cache usage
    if (GlancesAPI::fetchData("/api/4/fs", doc))
    {
        for (JsonVariant fs : doc.as<JsonArray>())
        {
            const char *mnt_point = fs["mnt_point"].as<const char *>();
            if (strcmp(mnt_point, "/rootfs/mnt/cache") == 0)
            {
                float usage = fs["percent"].as<float>();
                char buf[32];
                snprintf(buf, sizeof(buf), LV_SYMBOL_SAVE " Cache: %.1f%%", usage);
                update_compact_label(cache_label, buf);
                break;
            }
        }
    }

    // Uptime
    if (GlancesAPI::fetchData("/api/4/uptime", doc))
    {
        String payload = doc.as<String>();
        payload.replace("\"", "");
        char buf[32];
        snprintf(buf, sizeof(buf), LV_SYMBOL_POWER "  %s", payload.c_str());
        update_compact_label(uptime_label, buf);
    }

    // Network
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

    // Load
    if (GlancesAPI::fetchData("/api/4/load", doc))
    {
        float load1 = doc["min1"].as<float>();
        char buf[32];
        snprintf(buf, sizeof(buf), LV_SYMBOL_CHARGE " Load: %.1f", load1);
        update_compact_label(load_label, buf);
    }

    lastGlancesUpdate = millis();
}