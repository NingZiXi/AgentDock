idf_component_register(
    SRCS "src/bsp.c" "src/bsp_lcd.c" "src/bsp_touch.c"
    INCLUDE_DIRS "include"
    REQUIRES driver esp_lcd esp_timer
)