/**
 * @file bsp_pinmap.h
 * @brief Single source of truth for every GPIO on the AgentDock board.
 *
 * Change board → edit this file ONLY. Do not sprinkle GPIO_NUM_xx elsewhere.
 */
#pragma once
#include "driver/gpio.h"

/* LCD SPI */
#define BSP_PIN_LCD_CS        (GPIO_NUM_11)
#define BSP_PIN_LCD_DC        (GPIO_NUM_12)
#define BSP_PIN_LCD_SCLK      (GPIO_NUM_13)
#define BSP_PIN_LCD_MOSI      (GPIO_NUM_10)
#define BSP_PIN_LCD_RST       (GPIO_NUM_14)
/* TE pin defined but unused (no tearing support yet) */
#define BSP_PIN_LCD_TE        (GPIO_NUM_18)

/* LCD backlight (active high) */
#define BSP_PIN_LCD_BK        (GPIO_NUM_38)
#define BSP_LCD_BK_ON_LEVEL   (1)
#define BSP_LCD_BK_OFF_LEVEL  (0)

/* Touch I2C */
#define BSP_PIN_TOUCH_SCL     (GPIO_NUM_3)
#define BSP_PIN_TOUCH_SDA     (GPIO_NUM_46)
#define BSP_PIN_TOUCH_RST     (GPIO_NUM_8)
#define BSP_PIN_TOUCH_INT     (GPIO_NUM_9)
#define BSP_TOUCH_I2C_NUM     (0)
#define BSP_TOUCH_I2C_CLK_HZ  (400000)