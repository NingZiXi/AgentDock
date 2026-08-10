idf_component_register(SRCS "main.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES board lvgl_port agent_dock_ui esp_lv_eaf_player)

# Pack ../assets/ into a littlefs image and add it to the flash target so
# `idf.py flash` writes it to the `assets` partition automatically. Must be
# called AFTER idf_component_register so joltwallet's project_include.cmake
# and ESP-IDF's partition_table helpers are loaded.
littlefs_create_partition_image(assets ../assets FLASH_IN_PROJECT)