#include "gui.h"
#include "settings_manager.h"
#include <Arduino.h>
#include <stdio.h>

lv_obj_t *cpu_label = NULL;
lv_obj_t *ram_label = NULL;
lv_obj_t *disk_label = NULL;
lv_obj_t *uptime_label = NULL;
lv_obj_t *network_label = NULL;
lv_obj_t *cores_label = NULL;
lv_obj_t *total_ram_label = NULL;
lv_obj_t *temp_label = NULL;
lv_obj_t *load_label = NULL;
lv_obj_t *cache_label = NULL;
ArcWithLabel cpu_arc_obj = {NULL, NULL};
ArcWithLabel ram_arc_obj = {NULL, NULL};

ArcWithLabel create_arc(lv_obj_t *parent, const char *text, lv_color_t color)
{
    ArcWithLabel result = {nullptr, nullptr};
    if (!parent)
        return result;

    lv_obj_t *arc = lv_arc_create(parent);
    if (!arc)
        return result;

    lv_obj_set_size(arc, 110, 110);
    lv_arc_set_rotation(arc, 135);
    lv_arc_set_range(arc, 0, 100);
    lv_arc_set_bg_angles(arc, 0, 270);
    lv_arc_set_value(arc, 0);

    lv_obj_set_style_arc_color(arc, lv_color_darken(color, LV_OPA_30), LV_PART_MAIN);
    lv_obj_set_style_arc_width(arc, 6, LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc, color, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(arc, 6, LV_PART_INDICATOR);
    lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *cont = lv_obj_create(arc);
    lv_obj_remove_style_all(cont);
    lv_obj_set_size(cont, 90, 90);
    lv_obj_set_style_pad_all(cont, 0, 0);
    lv_obj_center(cont);

    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(cont, 2, 0);

    lv_obj_t *title = lv_label_create(cont);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(title, color, 0);
    lv_label_set_text(title, text);

    lv_obj_t *value = lv_label_create(cont);
    lv_obj_set_style_text_font(value, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(value, color, 0);
    lv_label_set_text(value, "--");

    lv_obj_t *info = lv_label_create(cont);
    lv_obj_set_style_text_font(info, &lv_font_montserrat_10, 0);
    lv_obj_set_style_text_color(info, lv_color_hex(0x808080), 0);
    lv_label_set_text(info, "--");

    lv_obj_t **labels = (lv_obj_t **)lv_mem_alloc(3 * sizeof(lv_obj_t *));
    labels[0] = title;
    labels[1] = value;
    labels[2] = info;
    lv_obj_set_user_data(arc, labels);

    result.arc = arc;
    result.label = cont;
    return result;
}

lv_obj_t *create_button_label(lv_obj_t *parent, const char *text, const ThemeColors *theme)
{
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_size(btn, 145, 30);
    lv_obj_set_style_radius(btn, 5, 0);
    lv_obj_set_style_bg_color(btn, theme->card_bg_color, 0);
    lv_obj_set_style_bg_opa(btn, LV_OPA_50, 0);
    lv_obj_set_style_border_width(btn, 1, 0);
    lv_obj_set_style_border_color(btn, theme->border_color, 0);
    lv_obj_set_style_shadow_width(btn, 5, 0);
    lv_obj_set_style_shadow_color(btn, lv_color_darken(theme->bg_color, LV_OPA_30), 0);
    lv_obj_set_style_pad_all(btn, 5, 0);

    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, text);
    lv_obj_center(label);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(label, theme->text_color, 0);

    return label;
}

lv_obj_t *create_compact_label(lv_obj_t *parent, const char *text, const ThemeColors *theme)
{
    lv_obj_t *btn = lv_obj_create(parent);
    lv_obj_set_size(btn, 154, LV_SIZE_CONTENT);
    lv_obj_set_style_radius(btn, 5, 0);
    lv_obj_set_style_bg_color(btn, theme->card_bg_color, 0);
    lv_obj_set_style_bg_opa(btn, LV_OPA_50, 0);
    lv_obj_set_style_border_width(btn, 1, 0);
    lv_obj_set_style_border_color(btn, theme->border_color, 0);
    lv_obj_set_style_shadow_width(btn, 5, 0);
    lv_obj_set_style_shadow_color(btn, lv_color_darken(theme->bg_color, LV_OPA_30), 0);
    lv_obj_set_style_pad_all(btn, 5, 0);

    if (strstr(text, LV_SYMBOL_DOWNLOAD) && strstr(text, LV_SYMBOL_UPLOAD))
    {
        lv_obj_t *text_label = lv_label_create(btn);
        lv_label_set_text(text_label, text);
        lv_obj_set_style_text_font(text_label, &lv_font_montserrat_14, 0);
        lv_obj_set_style_text_color(text_label, theme->text_color, 0);
        lv_obj_set_width(text_label, 140);
        lv_obj_set_style_text_align(text_label, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_align(text_label, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_user_data(btn, text_label);
        return btn;
    }

    if (!strstr(text, LV_SYMBOL_DOWNLOAD) || !strstr(text, LV_SYMBOL_UPLOAD))
    {
        lv_obj_set_flex_flow(btn, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(btn, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    }

    bool is_temp = strstr(text, "Temp:") != NULL;

    lv_obj_t *icon_label = lv_label_create(btn);
    char icon[32];
    const char *space_pos = strchr(text, ' ');
    if (space_pos)
    {
        size_t icon_len = space_pos - text;
        strncpy(icon, text, icon_len);
        icon[icon_len] = '\0';
    }
    else
    {
        strcpy(icon, "");
    }
    lv_label_set_text(icon_label, icon);
    lv_obj_set_style_text_font(icon_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(icon_label, theme->text_color, 0);

    lv_obj_t *text_label = lv_label_create(btn);
    if (space_pos)
    {
        const char *text_start = space_pos + 1;
        lv_label_set_text(text_label, text_start);
    }
    else
    {
        lv_label_set_text(text_label, "");
    }
    lv_obj_set_style_text_font(text_label, &lv_font_montserrat_14, 0);
    if (!strstr(text, "Temp:"))
    {
        lv_obj_set_style_text_color(text_label, theme->text_color, 0);
    }
    lv_obj_set_user_data(btn, text_label);

    return btn;
}

void update_compact_label(lv_obj_t *btn, const char *text)
{
    lv_obj_t *text_label = (lv_obj_t *)lv_obj_get_user_data(btn);

    if (strstr(text, LV_SYMBOL_DOWNLOAD) && strstr(text, LV_SYMBOL_UPLOAD))
    {
        lv_label_set_text(text_label, text);
        return;
    }

    if (strstr(text, "Temp:"))
    {
        const char *temp_start = strstr(text, "Temp:") + 5;
        int temperature = 0;

        if (sscanf(temp_start, "%d", &temperature) == 1)
        {
            lv_color_t temp_color;
            if (temperature < 40)
            {
                temp_color = lv_color_hex(0x00FF44);
            }
            else if (temperature < 50)
            {
                temp_color = lv_color_hex(0xFFAA00);
            }
            else
            {
                temp_color = lv_color_hex(0xFF4444);
            }

            lv_obj_set_style_text_color(text_label, temp_color, 0);
        }
        else
        {
            Serial.println("Failed to parse temperature!");
        }

        lv_label_set_text(text_label, temp_start);
        return;
    }

    const char *space_pos = strchr(text, ' ');
    if (space_pos)
    {
        const char *text_start = space_pos + 1;
        lv_label_set_text(text_label, text_start);
    }
}

void set_arc_value_animated(lv_obj_t *arc, int32_t value, uint32_t duration)
{
    if (!arc)
        return;

    value = (value < 0) ? 0 : (value > 100) ? 100
                                            : value;

    static lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, arc);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_arc_set_value);
    lv_anim_set_values(&a, lv_arc_get_value(arc), value);
    lv_anim_set_time(&a, duration);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
    lv_anim_set_ready_cb(&a, NULL);
    lv_anim_start(&a);
}

void update_arc_label(lv_obj_t *label, const char *text)
{
    if (!label || !text)
        return;
    lv_label_set_text(label, text);
}

void applyTheme(bool darkMode)
{
    const ThemeColors &theme = SettingsManager::getCurrentTheme();
    lv_obj_set_style_bg_color(lv_scr_act(), theme.bg_color, 0);

    if (cpu_arc_obj.arc)
    {
        lv_obj_set_style_arc_color(cpu_arc_obj.arc, theme.cpu_color, LV_PART_INDICATOR);
        lv_obj_set_style_arc_color(cpu_arc_obj.arc, lv_color_darken(theme.cpu_color, LV_OPA_30), LV_PART_MAIN);
        lv_obj_t **labels = (lv_obj_t **)lv_obj_get_user_data(cpu_arc_obj.arc);
        if (labels)
        {
            lv_obj_set_style_text_color(labels[0], theme.text_color, 0);
            lv_obj_set_style_text_color(labels[1], theme.text_color, 0);
            lv_obj_set_style_text_color(labels[2], lv_color_hex(0x808080), 0);
        }
    }

    if (ram_arc_obj.arc)
    {
        lv_obj_set_style_arc_color(ram_arc_obj.arc, theme.ram_color, LV_PART_INDICATOR);
        lv_obj_set_style_arc_color(ram_arc_obj.arc, lv_color_darken(theme.ram_color, LV_OPA_30), LV_PART_MAIN);

        lv_obj_t **labels = (lv_obj_t **)lv_obj_get_user_data(ram_arc_obj.arc);
        if (labels)
        {

            lv_obj_set_style_text_color(labels[0], theme.text_color, 0);
            lv_obj_set_style_text_color(labels[1], theme.text_color, 0);

            lv_obj_set_style_text_color(labels[2], lv_color_hex(0x808080), 0);
        }
    }

    lv_obj_t *compact_labels[] = {
        temp_label,
        load_label,
        uptime_label,
        disk_label,
        cache_label,
        network_label};

    for (lv_obj_t *label : compact_labels)
    {
        if (label)
        {
            lv_obj_set_style_border_color(label, theme.border_color, 0);
            lv_obj_set_style_bg_color(label, theme.card_bg_color, 0);
            lv_obj_t *icon_label = lv_obj_get_child(label, 0);
            if (icon_label)
            {
                lv_obj_set_style_text_color(icon_label, theme.text_color, 0);
            }
            lv_obj_t *text_label = (lv_obj_t *)lv_obj_get_user_data(label);
            if (text_label)
            {
                if (!strstr(lv_label_get_text(text_label), "°C"))
                {
                    lv_obj_set_style_text_color(text_label, theme.text_color, 0);
                }
            }
        }
    }
}

void create_system_monitor_gui()
{
    Serial.println("Creating system monitor GUI...");
    
    const ThemeColors *theme = DARK_MODE ? &dark_theme : &light_theme;
    lv_obj_set_style_bg_color(lv_scr_act(), theme->bg_color, 0);

    lv_obj_t *main_cont = lv_obj_create(lv_scr_act());
    if (!main_cont) {
        Serial.println("Failed to create main container");
        return;
    }

    lv_obj_set_size(main_cont, 320, 240);
    lv_obj_set_style_pad_all(main_cont, 2, 0);
    lv_obj_set_style_bg_opa(main_cont, 0, 0);
    lv_obj_set_style_border_width(main_cont, 0, 0);
    lv_obj_set_flex_flow(main_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(main_cont, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_clear_flag(main_cont, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *left_col = lv_obj_create(main_cont);
    if (!left_col) {
        Serial.println("Failed to create left column");
        return;
    }

    lv_obj_t *right_col = lv_obj_create(main_cont);
    if (!right_col) {
        Serial.println("Failed to create right column");
        return;
    }

    lv_obj_set_size(left_col, 158, 240);
    lv_obj_set_style_pad_all(left_col, 2, 0);
    lv_obj_set_style_bg_opa(left_col, LV_OPA_0, 0);
    lv_obj_set_style_border_width(left_col, 0, 0);
    lv_obj_set_flex_flow(left_col, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(left_col, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_set_size(right_col, 158, 240);
    lv_obj_set_style_pad_all(right_col, 2, 0);
    lv_obj_set_style_bg_opa(right_col, LV_OPA_0, 0);
    lv_obj_set_style_border_width(right_col, 0, 0);
    lv_obj_set_flex_flow(right_col, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(right_col, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    Serial.println("Creating CPU arc...");
    cpu_arc_obj = create_arc(left_col, "CPU", theme->cpu_color);
    if (!cpu_arc_obj.arc || !cpu_arc_obj.label) {
        Serial.println("Failed to create CPU arc");
        return;
    }

    Serial.println("Creating RAM arc...");
    ram_arc_obj = create_arc(right_col, "RAM", theme->ram_color);
    if (!ram_arc_obj.arc || !ram_arc_obj.label) {
        Serial.println("Failed to create RAM arc");
        return;
    }

    temp_label = create_compact_label(left_col, LV_SYMBOL_WARNING " Temp: -- °C", theme);
    if (!temp_label) {
        Serial.println("Failed to create temp label");
        return;
    }

    load_label = create_compact_label(left_col, LV_SYMBOL_CHARGE " Load: -.-", theme);
    if (!load_label) {
        Serial.println("Failed to create load label");
        return;
    }

    uptime_label = create_compact_label(left_col, LV_SYMBOL_POWER "  ---", theme);
    if (!uptime_label) {
        Serial.println("Failed to create uptime label");
        return;
    }

    disk_label = create_compact_label(right_col, LV_SYMBOL_DRIVE " Array: ---%", theme);
    if (!disk_label) {
        Serial.println("Failed to create disk label");
        return;
    }

    cache_label = create_compact_label(right_col, LV_SYMBOL_SAVE " Cache: ---%", theme);
    if (!cache_label) {
        Serial.println("Failed to create cache label");
        return;
    }

    network_label = create_compact_label(right_col, LV_SYMBOL_DOWNLOAD " --- " LV_SYMBOL_UPLOAD " ---", theme);
    if (!network_label) {
        Serial.println("Failed to create network label");
        return;
    }

    SettingsManager::setThemeChangeCallback(applyTheme);
    applyTheme(SettingsManager::getDarkMode());
    
    Serial.println("GUI creation completed successfully!");
}