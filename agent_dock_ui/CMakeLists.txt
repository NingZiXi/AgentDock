# Auto-pick up every .c / .cpp that EEZ Studio exports into src/ui/.
# CONFIGURE_DEPENDS is not usable here because ESP-IDF's component
# CMake script re-includes this file in script mode, where the
# CONFIGURE_DEPENDS keyword is rejected. Re-run `idf.py reconfigure`
# (or just rebuild) after EEZ adds new files.
file(GLOB AGENT_DOCK_UI_SOURCES
    "src/ui/*.c"
    "src/ui/*.cpp")

idf_component_register(
    SRCS ${AGENT_DOCK_UI_SOURCES}
    # ui.h, screens.h, images.h, etc. all live under src/ui/, so any
    # component that REQUIRES agent_dock_ui gets this path automatically
    # and can `#include "ui.h"`.
    INCLUDE_DIRS "src/ui"
    REQUIRES lvgl_port
)