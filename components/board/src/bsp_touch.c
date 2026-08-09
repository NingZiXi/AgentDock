/**
 * @file bsp_touch.c
 * @brief Touch BSP — I2C master bus + AXS15231B touch controller.
 *
 * Coordinates are already swap_xy/mirror_x corrected in this layer so
 * consumers (lvgl_port, UI apps) see screen-aligned (x,y).
 *
 * bsp_touch_deinit() properly releases I2C bus (fixes the leak in the
 * pre-refactor main/lvgl_port.c).
 */

#include "bsp/bsp.h"
#include "bsp/bsp_pinmap.h"
#include "bsp/bsp_lcd.h"          /* BSP_LCD_H_RES / BSP_LCD_V_RES for touch coordinate axis */
#include "bsp/bsp_touch.h"
#include "driver/i2c_master.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_axs15231b.h"
#include "esp_err.h"
#include "esp_log.h"

static const char *TAG = "bsp_touch";

static i2c_master_bus_handle_t   s_i2c_bus = NULL;
static esp_lcd_panel_io_handle_t s_tp_io   = NULL;
static esp_lcd_touch_handle_t    s_touch   = NULL;
static bool                      s_inited  = false;

static void touch_process_points_cb(esp_lcd_touch_handle_t tp,
                                    uint16_t *x, uint16_t *y,
                                    uint16_t *strength,
                                    uint8_t *point_num,
                                    uint8_t max_point_num)
{
    /* IC raw axes are both mirrored — flip both. */
    for (int i = 0; i < *point_num; i++) {
        x[i] = tp->config.x_max - x[i];
        y[i] = tp->config.y_max - y[i];
    }
}

esp_err_t bsp_touch_init(esp_lcd_touch_handle_t *out_touch)
{
    if (out_touch == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    if (s_inited) {
        /* Idempotent: return existing handle (see bsp_lcd_init). */
        *out_touch = s_touch;
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Initialize I2C bus (touch)");
    const i2c_master_bus_config_t i2c_cfg = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port   = BSP_TOUCH_I2C_NUM,
        .scl_io_num = BSP_PIN_TOUCH_SCL,
        .sda_io_num = BSP_PIN_TOUCH_SDA,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_cfg, &s_i2c_bus));

    /* NOTE: cannot use ESP_LCD_TOUCH_IO_I2C_AXS15231B_CONFIG_EX(scl_speed_hz) —
     * its parameter name `scl_speed_hz` collides with the struct field name in the
     * macro body, so any non-trivial argument causes the field name itself to be
     * macro-substituted and the designated initializer breaks. Workaround:
     * use the no-EX macro, then set the field manually. */
    esp_lcd_panel_io_i2c_config_t tp_io_cfg = ESP_LCD_TOUCH_IO_I2C_AXS15231B_CONFIG();
    tp_io_cfg.scl_speed_hz = BSP_TOUCH_I2C_CLK_HZ;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(s_i2c_bus, &tp_io_cfg, &s_tp_io));

    esp_lcd_touch_config_t tp_cfg = {
        .x_max        = BSP_LCD_V_RES,
        .y_max        = BSP_LCD_H_RES,
        .rst_gpio_num = BSP_PIN_TOUCH_RST,
        .int_gpio_num = BSP_PIN_TOUCH_INT,
        .process_coordinates = touch_process_points_cb,
        .levels = { .reset = 0, .interrupt = 0 },
        .flags  = { .swap_xy = 0, .mirror_x = 0, .mirror_y = 0 },
    };
    ESP_LOGI(TAG, "Initialize touch controller AXS15231B");
    ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_axs15231b(s_tp_io, &tp_cfg, &s_touch));

    *out_touch = s_touch;
    s_inited = true;
    return ESP_OK;
}

esp_err_t bsp_touch_deinit(void)
{
    if (!s_inited) {
        return ESP_ERR_INVALID_STATE;
    }
    /* Order: touch → touch IO → I2C bus */
    if (s_touch) {
        esp_lcd_touch_del(s_touch);
        s_touch = NULL;
    }
    if (s_tp_io) {
        esp_lcd_panel_io_del(s_tp_io);
        s_tp_io = NULL;
    }
    if (s_i2c_bus) {
        i2c_del_master_bus(s_i2c_bus);
        s_i2c_bus = NULL;
    }
    s_inited = false;
    return ESP_OK;
}