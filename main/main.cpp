// main.cpp — entry point for the AgentDock firmware.

#include <stdio.h>
#include "esp_log.h"
#include "bsp/bsp.h"
#include "lvgl_port.h"
#include "ui.h"
#include "lvgl.h"

static const char *TAG = "main";

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "AgentDock starting...");
    ESP_ERROR_CHECK(bsp_init());              /* 1) Board hardware (LCD + touch) */
    ESP_ERROR_CHECK(lvgl_port_init());        /* 2) LVGL display + tick + task */

    ui_init();                                           /* create EEZ screens/objects */
    lv_timer_create((lv_timer_cb_t)ui_tick, 33, NULL);   /* tick EEZ flow each LVGL frame */

    lv_sysmon_show_performance(lvgl_port_get_display());

    ESP_LOGI(TAG, "EEZ UI running.");
}