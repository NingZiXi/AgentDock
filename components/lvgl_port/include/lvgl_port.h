/**
 * @file lvgl_port.h
 * @brief LVGL port — bring up display + touch input + tick + task in one call.
 *
 * Pulls LCD / touch handles from the BSP layer; this component knows nothing
 * about pins, SPI, I2C or the AXS15231B controller.
 */

#pragma once

#include "esp_err.h"
#include "lvgl.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Bring up LVGL display + touch input + tick + task. Idempotent. */
esp_err_t lvgl_port_init(void);

/** Tear everything down. */
esp_err_t lvgl_port_deinit(void);

/** The LVGL display. NULL before init or after deinit. */
lv_display_t *lvgl_port_get_display(void);

/** Lock the LVGL API for cross-thread access. Not for ISR. */
bool lvgl_port_lock(uint32_t timeout_ms);
void lvgl_port_unlock(void);

#ifdef __cplusplus
}
#endif