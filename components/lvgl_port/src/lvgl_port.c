/**
 * @file lvgl_port.c
 * @brief LVGL port — only LVGL glue. All hardware init lives in BSP.
 *
 * Acquire LCD / touch handles from bsp_lcd_init() + bsp_touch_init(), then
 * build the LVGL display / buffer / flush callback / tick timer / indev /
 * task on top.
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/lock.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_lcd_panel_ops.h"
#include "esp_err.h"
#include "esp_log.h"
#include "lvgl.h"

#include "bsp/bsp.h"
#include "bsp/bsp_lcd.h"
#include "bsp/bsp_touch.h"
#include "lvgl_port.h"

static const char *TAG = "lvgl_port";

/* LVGL task tuning */
#define LVGL_TICK_PERIOD_MS    (2)
#define LVGL_TASK_MAX_DELAY_MS (500)
#define LVGL_TASK_MIN_DELAY_MS (1000 / CONFIG_FREERTOS_HZ)
#define LVGL_TASK_STACK_SIZE   (8 * 1024)
#define LVGL_TASK_PRIORITY     (2)

/* Module state */
static _lock_t                g_api_lock;
static bool                   g_initialized   = false;
static lv_display_t          *g_display       = NULL;
static lv_indev_t            *g_indev         = NULL;
static esp_timer_handle_t     g_tick_timer    = NULL;
static TaskHandle_t           g_task_handle   = NULL;
static lv_color16_t          *g_buf1          = NULL;
static lv_color16_t          *g_buf2          = NULL;

/* ─── LVGL ↔ driver glue ──────────────────────────────────────────────── */

static bool notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t panel_io,
                                    esp_lcd_panel_io_event_data_t *edata,
                                    void *user_ctx)
{
    lv_display_flush_ready((lv_display_t *)user_ctx);
    return false;
}

// 软件 90° CW 旋转输出缓冲
static lv_color16_t *s_rot_buf = NULL;
static size_t         s_rot_buf_sz = 0;

static void lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    esp_lcd_panel_handle_t panel = lv_display_get_user_data(disp);

    const int x1 = area->x1, x2 = area->x2;
    const int y1 = area->y1, y2 = area->y2;
    const int src_w = x2 - x1 + 1;
    const int src_h = y2 - y1 + 1;
    const size_t needed = (size_t)src_w * (size_t)src_h * sizeof(lv_color16_t);

    if (s_rot_buf == NULL || needed > s_rot_buf_sz) {
        free(s_rot_buf);
        s_rot_buf = heap_caps_malloc(needed, MALLOC_CAP_SPIRAM);
        assert(s_rot_buf);
        s_rot_buf_sz = needed;
    }

    // 90° CW: src(sx,sy) → dst(py=sx, px=src_h-1-sy)
    const lv_color16_t *src = (const lv_color16_t *)px_map;
    lv_color16_t       *dst = s_rot_buf;
    for (int py = 0; py < src_w; py++) {
        for (int px = 0; px < src_h; px++) {
            dst[py * src_h + px] = src[(src_h - 1 - px) * src_w + py];
        }
    }

    lv_draw_sw_rgb565_swap((uint8_t *)dst, (size_t)src_w * src_h);

    const int phys_x_start = BSP_LCD_H_RES - 1 - y2;
    const int phys_x_end   = BSP_LCD_H_RES - 1 - y1 + 1;
    const int phys_y_start = x1;
    const int phys_y_end   = x2 + 1;
    esp_lcd_panel_draw_bitmap(panel, phys_x_start, phys_y_start,
                              phys_x_end, phys_y_end, dst);
}

static void lvgl_tick_cb(void *arg)
{
    lv_tick_inc(LVGL_TICK_PERIOD_MS);
}

static void touch_read_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
    esp_lcd_touch_handle_t tp = lv_indev_get_user_data(indev);
    esp_lcd_touch_point_data_t point;
    uint8_t tp_cnt = 0;
    esp_lcd_touch_read_data(tp);
    esp_err_t err = esp_lcd_touch_get_data(tp, &point, &tp_cnt, 1);
    if (err == ESP_OK && tp_cnt > 0) {
        data->point.x = point.x;
        data->point.y = point.y;
        data->state = LV_INDEV_STATE_PRESSED;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

static void lvgl_task(void *arg)
{
    ESP_LOGI(TAG, "LVGL task running");
    uint32_t next_ms;
    while (1) {
        _lock_acquire(&g_api_lock);
        next_ms = lv_timer_handler();
        _lock_release(&g_api_lock);
        next_ms = MAX(next_ms, LVGL_TASK_MIN_DELAY_MS);
        next_ms = MIN(next_ms, LVGL_TASK_MAX_DELAY_MS);
        usleep(next_ms * 1000);
    }
}

/* ─── Public API ──────────────────────────────────────────────────────── */

esp_err_t lvgl_port_init(void)
{
    if (g_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    /* 1) Get hardware handles from BSP */
    esp_lcd_panel_io_handle_t io = NULL;
    esp_lcd_panel_handle_t panel = NULL;
    ESP_ERROR_CHECK(bsp_lcd_init(&io, &panel));

    esp_lcd_touch_handle_t tp = NULL;
    ESP_ERROR_CHECK(bsp_touch_init(&tp));

    /* 2) LVGL display + PSRAM buffers + flush cb */
    ESP_LOGI(TAG, "Initialize LVGL");
    lv_init();

    g_display = lv_display_create(BSP_LCD_V_RES, BSP_LCD_H_RES);
    size_t buf_sz = BSP_LCD_V_RES * BSP_LCD_DRAW_BUFF_HEIGHT * sizeof(lv_color16_t);
    g_buf1 = heap_caps_malloc(buf_sz, MALLOC_CAP_SPIRAM);
    g_buf2 = heap_caps_malloc(buf_sz, MALLOC_CAP_SPIRAM);
    assert(g_buf1 && g_buf2);
    lv_display_set_buffers(g_display, g_buf1, g_buf2, buf_sz, LV_DISPLAY_RENDER_MODE_FULL);
    lv_display_set_user_data(g_display, panel);
    lv_display_set_color_format(g_display, LV_COLOR_FORMAT_RGB565);
    lv_display_set_flush_cb(g_display, lvgl_flush_cb);

    /* 3) LVGL tick timer */
    const esp_timer_create_args_t tick_args = {
        .callback = lvgl_tick_cb,
        .name     = "lvgl_tick",
    };
    ESP_ERROR_CHECK(esp_timer_create(&tick_args, &g_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(g_tick_timer, LVGL_TICK_PERIOD_MS * 1000));

    /* 4) Flush-ready callback */
    const esp_lcd_panel_io_callbacks_t cbs = {
        .on_color_trans_done = notify_lvgl_flush_ready,
    };
    ESP_ERROR_CHECK(esp_lcd_panel_io_register_event_callbacks(io, &cbs, g_display));

    /* 5) Touch indev */
    g_indev = lv_indev_create();
    lv_indev_set_type(g_indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_display(g_indev, g_display);
    lv_indev_set_user_data(g_indev, tp);
    lv_indev_set_read_cb(g_indev, touch_read_cb);

    /* 6) LVGL task */
    if (xTaskCreate(lvgl_task, "LVGL", LVGL_TASK_STACK_SIZE, NULL,
                    LVGL_TASK_PRIORITY, &g_task_handle) != pdPASS) {
        return ESP_FAIL;
    }

    g_initialized = true;
    return ESP_OK;
}

esp_err_t lvgl_port_deinit(void)
{
    if (!g_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    if (g_task_handle) {
        vTaskDelete(g_task_handle);
        g_task_handle = NULL;
    }
    if (g_tick_timer) {
        esp_timer_stop(g_tick_timer);
        esp_timer_delete(g_tick_timer);
        g_tick_timer = NULL;
    }
    if (g_indev) {
        lv_indev_delete(g_indev);
        g_indev = NULL;
    }

    free(g_buf1);
    free(g_buf2);
    free(s_rot_buf);
    s_rot_buf = NULL;
    s_rot_buf_sz = 0;
    g_buf1 = g_buf2 = NULL;
    g_display = NULL;

    /* Release hardware (BSP) — touch first, then LCD */
    ESP_ERROR_CHECK(bsp_touch_deinit());
    ESP_ERROR_CHECK(bsp_lcd_deinit());

    g_initialized = false;
    return ESP_OK;
}

lv_display_t *lvgl_port_get_display(void)
{
    return g_display;
}

bool lvgl_port_lock(uint32_t timeout_ms)
{
    (void)timeout_ms;
    _lock_acquire(&g_api_lock);
    return true;
}

void lvgl_port_unlock(void)
{
    _lock_release(&g_api_lock);
}