/**
 * @file bsp_lcd.h
 * @brief LCD hardware abstraction — initialise the SPI bus, panel IO and
 *        AXS15231B controller, plus backlight control.
 *
 * Consumers (e.g. lvgl_port) get back esp_lcd_panel_io_handle_t and
 * esp_lcd_panel_handle_t — they don't need to know what chip is behind them.
 */
#pragma once
#include "esp_err.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include <stdbool.h>

#define BSP_LCD_H_RES              (170)
#define BSP_LCD_V_RES              (560)
#define BSP_LCD_PIXEL_CLK_HZ       (80 * 1000 * 1000)
#define BSP_LCD_SPI_NUM            (SPI2_HOST)
#define BSP_LCD_COLOR_DEPTH        (16)
#define BSP_LCD_DRAW_BUFF_HEIGHT   (BSP_LCD_V_RES)

/** Init SPI bus + panel IO + AXS15231B + backlight (turned on).
 *  Idempotent. Returns ESP_ERR_INVALID_STATE if already initialised. */
esp_err_t bsp_lcd_init(esp_lcd_panel_io_handle_t *out_io,
                       esp_lcd_panel_handle_t *out_panel);

esp_err_t bsp_lcd_deinit(void);

/** Set backlight on/off (no-op if deinit'd). */
esp_err_t bsp_lcd_set_backlight(bool on);