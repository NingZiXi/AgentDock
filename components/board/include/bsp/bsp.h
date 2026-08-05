/**
 * @file bsp.h
 * @brief Board Support Package top-level entry.
 *
 * bsp_init() brings up all on-board hardware. After it returns, the LCD is on
 * and the touch controller is polled. Idempotent.
 */
#pragma once
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t bsp_init(void);
esp_err_t bsp_deinit(void);

#ifdef __cplusplus
}
#endif