#ifndef DISPLAY_H
#define DISPLAY_H

#include <lvgl.h>
#include <TFT_eSPI.h>

extern TFT_eSPI tft;
extern lv_disp_draw_buf_t draw_buf;
extern lv_color_t *buf1;
extern lv_color_t *buf2;

void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
void init_display();

#endif