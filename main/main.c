#include <stdio.h>
#include "esp_log.h"
#include "bsp/bsp.h"
#include "lvgl_port.h"

static const char *TAG = "main";

/**
 * User UI hook — add widgets to lv_screen_active() here.
 * Simple smoke test: centred label on the active screen.
 */
static void ui_init(void)
{
    lv_obj_t *label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "Hello AgentDock");
    lv_obj_center(label);
}

void app_main(void)
{
    ESP_LOGI(TAG, "AgentDock starting...");
    ESP_ERROR_CHECK(bsp_init());          /* 1) Board hardware (LCD + touch) */
    ESP_ERROR_CHECK(lvgl_port_init());    /* 2) LVGL display + tick + task */
    ui_init();                            /* 3) User UI */
    ESP_LOGI(TAG, "BSP + LVGL ready. Add widgets in ui_init().");
}