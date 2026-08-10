// main.cpp — AgentDock 固件入口

#include <stdio.h>
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_littlefs.h"
#include "bsp/bsp.h"
#include "lvgl_port.h"
#include "ui.h"
#include "lvgl.h"
#include "lv_eaf.h"

static const char *TAG = "main";

// PSRAM 中 mascot.eaf 的常驻 buffer — EAF widget 持引用直到切源
static uint8_t *s_mascot_buf = NULL;

static void mount_mascot(void)
{
    FILE *f = fopen("/assets/mascot.eaf", "rb");
    if (!f) {
        ESP_LOGE(TAG, "open /assets/mascot.eaf failed");
        return;
    }
    fseek(f, 0, SEEK_END);
    size_t sz = ftell(f);
    rewind(f);
    s_mascot_buf = (uint8_t *)heap_caps_malloc(sz, MALLOC_CAP_SPIRAM);
    if (!s_mascot_buf) {
        ESP_LOGE(TAG, "malloc %u for mascot failed", (unsigned)sz);
        fclose(f);
        return;
    }
    fread(s_mascot_buf, 1, sz, f);
    fclose(f);
    ESP_LOGI(TAG, "mascot.eaf loaded: %u bytes", (unsigned)sz);

    lv_obj_t *mascot = lv_eaf_create(lv_screen_active());
    lv_eaf_set_src_data(mascot, s_mascot_buf, sz);
    lv_eaf_set_loop_count(mascot, -1);
    lv_eaf_set_frame_delay(mascot, 250);
    lv_obj_align(mascot, LV_ALIGN_RIGHT_MID, -10, 0);
}

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "AgentDock starting...");

    // 挂载 littlefs 到 assets 分区,首次启动失败时 format
    esp_vfs_littlefs_conf_t fs_conf = {
        .base_path = "/assets",
        .partition_label = "assets",
        .partition = NULL,
        .blockdev = NULL,
        .format_if_mount_failed = true,
        .read_only = false,
        .dont_mount = false,
        .grow_on_mount = false,
    };
    ESP_ERROR_CHECK(esp_vfs_littlefs_register(&fs_conf));

    ESP_ERROR_CHECK(bsp_init());
    ESP_ERROR_CHECK(lvgl_port_init());

    ui_init();
    lv_timer_create((lv_timer_cb_t)ui_tick, 33, NULL);

    mount_mascot();
    lv_sysmon_show_performance(lvgl_port_get_display());

    ESP_LOGI(TAG, "EEZ UI running.");
}