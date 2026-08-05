/**
 * @file bsp_touch.h
 * @brief Touch controller abstraction — initialise the I2C master bus and
 *        the AXS15231B touch IC. Coordinates are already swap_xy/mirror_x
 *        corrected in BSP so consumers see screen-aligned (x,y).
 */
#pragma once
#include "esp_err.h"
#include "esp_lcd_touch.h"

/** Init I2C master bus + AXS15231B touch controller. */
esp_err_t bsp_touch_init(esp_lcd_touch_handle_t *out_touch);

/** Release touch + touch IO + I2C bus. */
esp_err_t bsp_touch_deinit(void);