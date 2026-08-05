#include "bsp/bsp.h"
#include "bsp/bsp_lcd.h"
#include "bsp/bsp_touch.h"
#include "esp_log.h"

static const char *TAG = "bsp";
static bool s_inited = false;

esp_err_t bsp_init(void)
{
    if (s_inited) {
        return ESP_ERR_INVALID_STATE;
    }
    ESP_LOGI(TAG, "Init board hardware");

    esp_lcd_panel_io_handle_t io = NULL;
    esp_lcd_panel_handle_t panel = NULL;
    ESP_ERROR_CHECK(bsp_lcd_init(&io, &panel));

    esp_lcd_touch_handle_t tp = NULL;
    ESP_ERROR_CHECK(bsp_touch_init(&tp));

    s_inited = true;
    return ESP_OK;
}

esp_err_t bsp_deinit(void)
{
    if (!s_inited) {
        return ESP_ERR_INVALID_STATE;
    }
    ESP_ERROR_CHECK(bsp_touch_deinit());
    ESP_ERROR_CHECK(bsp_lcd_deinit());
    s_inited = false;
    return ESP_OK;
}