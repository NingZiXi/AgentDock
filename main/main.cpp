// main.cpp — entry point for the AgentDock firmware.
// TEMP: hand-written UI demo to verify BSP/LVGL orientation BEFORE
// wiring in EEZ. Replace with EEZ once screen direction is confirmed.

#include <stdio.h>
#include "esp_log.h"
#include "bsp/bsp.h"
#include "lvgl_port.h"

static const char *TAG = "main";

static void build_demo_ui(void)
{
    // Background: dark blue — easy to spot display issues
    lv_obj_set_style_bg_color(lv_screen_active(),
                              lv_color_hex(0x003a70), 0);
    lv_obj_set_style_bg_opa(lv_screen_active(), LV_OPA_COVER, 0);
    lv_obj_set_style_text_color(lv_screen_active(),
                                lv_color_hex(0xffffff), 0);

    // Edge labels — tell you which physical edge of the panel is which
    lv_obj_t *top    = lv_label_create(lv_screen_active());
    lv_label_set_text(top, "TOP EDGE");
    lv_obj_align(top, LV_ALIGN_TOP_MID, 0, 0);

    lv_obj_t *bottom = lv_label_create(lv_screen_active());
    lv_label_set_text(bottom, "BOTTOM EDGE");
    lv_obj_align(bottom, LV_ALIGN_BOTTOM_MID, 0, 0);

    lv_obj_t *left = lv_label_create(lv_screen_active());
    lv_label_set_text(left, "LEFT");
    lv_obj_align(left, LV_ALIGN_LEFT_MID, 0, 0);

    lv_obj_t *right = lv_label_create(lv_screen_active());
    lv_label_set_text(right, "RIGHT");
    lv_obj_align(right, LV_ALIGN_RIGHT_MID, 0, 0);

    lv_obj_t *cx = lv_label_create(lv_screen_active());
    lv_label_set_text(cx, "+ CENTER");
    lv_obj_align(cx, LV_ALIGN_CENTER, 0, 0);

    // Resolution readout (cast to int — values fit)
    lv_disp_t *disp = lvgl_port_get_display();
    lv_obj_t *res = lv_label_create(lv_screen_active());
    lv_label_set_text_fmt(res, "LVGL display: %d x %d",
                          (int)lv_display_get_horizontal_resolution(disp),
                          (int)lv_display_get_vertical_resolution(disp));
    lv_obj_align(res, LV_ALIGN_BOTTOM_MID, 0, -16);
}

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "AgentDock demo starting...");
    ESP_ERROR_CHECK(bsp_init());              /* 1) Board hardware (LCD + touch) */
    ESP_ERROR_CHECK(lvgl_port_init());        /* 2) LVGL display + tick + task */

    build_demo_ui();

    ESP_LOGI(TAG, "Demo UI built. Touch the screen to verify touch.");
}