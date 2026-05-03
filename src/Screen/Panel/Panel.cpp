#include "Panel.h"

#include <esp32_smartdisplay.h>
#include <lvgl/lvgl.h>
#include <ui/ui.h>

namespace {
bool initialized = false;
uint32_t lastTickMs = 0;
}

void Panel::init() {
    if (initialized) return;

    smartdisplay_init();
    smartdisplay_lcd_set_backlight(1.0f);

    ui_init();

    lastTickMs = millis();
    initialized = true;
}

void Panel::process() {
    if (!initialized) return;

    const uint32_t now = millis();
    lv_tick_inc(now - lastTickMs);
    lastTickMs = now;

    lv_timer_handler();
    ui_tick();
}

bool Panel::isInitialized() {
    return initialized;
}
