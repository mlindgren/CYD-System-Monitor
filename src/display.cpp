#include "display.h"
#include <Arduino.h>

#define TFT_BL 21  // Correct backlight pin for CYD
#define TFT_BACKLIGHT_ON HIGH

TFT_eSPI tft = TFT_eSPI();
lv_disp_draw_buf_t draw_buf;
lv_color_t *buf1;
lv_color_t *buf2;

void init_display()
{
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, TFT_BACKLIGHT_ON);

    tft.begin();
    tft.setRotation(3);  // Landscape orientation
    tft.initDMA();
    tft.fillScreen(TFT_BLACK);

    // Allocate display buffers - fix the width/height issue
    extern const uint16_t screenHeight;
    extern const uint16_t screenWidth;
    
    // For landscape mode (rotation 3), actual dimensions are swapped
    uint16_t buffer_width = screenWidth;   // 240
    uint16_t buffer_height = screenHeight; // 320
    
    // Allocate buffers with proper size calculation
    size_t buffer_size = buffer_width * 10; // 10 lines buffer
    buf1 = (lv_color_t *)heap_caps_malloc(sizeof(lv_color_t) * buffer_size, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    buf2 = (lv_color_t *)heap_caps_malloc(sizeof(lv_color_t) * buffer_size, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    
    if (buf1 == NULL || buf2 == NULL) {
        Serial.println("Failed to allocate display buffers!");
        return;
    }
    
    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, buffer_size);

    // Initialize display driver with correct resolution
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = screenWidth;   // 240
    disp_drv.ver_res = screenHeight;  // 320
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);
    
    Serial.println("Display initialized successfully");
}

void display_sleep(bool sleep)
{
    if (sleep)
    {
        digitalWrite(TFT_BL, !TFT_BACKLIGHT_ON);
        tft.writecommand(0x28);
        delay(100);
        tft.writecommand(0x10);
    }
    else
    {
        tft.writecommand(0x11);
        delay(100);
        tft.writecommand(0x29);
        delay(100);
        digitalWrite(TFT_BL, TFT_BACKLIGHT_ON);
    }
}

void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    tft.startWrite();
    tft.pushImageDMA(area->x1, area->y1, w, h, (uint16_t *)color_p);
    tft.endWrite();

    lv_disp_flush_ready(disp);
}