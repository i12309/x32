# Этап 01: подключить дисплей, LVGL и generated UI

## Цель

Получить минимальную прошивку, где на `JC8048W550C` запускается `esp32-smartdisplay`, создаются EEZ-экраны и показывается `Load0` или `Load`.

## Работы

1. Обновить `platformio.ini`:
   - проверить, что локальные библиотеки `lib/lvgl`, `lib/eez_ui` и `lib/esp32-smartdisplay` попадают в dependency graph после подключения bootstrap-кода;
   - при проблемах с поиском `lv_conf.h` добавить `build_flags` для `LV_CONF_PATH` на `include/lv_conf.h`;
   - не добавлять старый `include/ui/vendor` после переноса LVGL в `lib/lvgl`.
2. Решить include-совместимость LVGL:
   - текущий generated EEZ code использует `lvgl/lvgl.h`;
   - в `lib/lvgl/lvgl/lvgl.h` добавлен compatibility header, который подключает `../lvgl.h`;
   - в будущем можно поменять настройку EEZ `lvglInclude` с `lvgl/lvgl.h` на `lvgl.h` и регенерировать.
3. Создать `src/Screen/Panel/Panel.h|cpp`:
   - `init()` вызывает `smartdisplay_init()`, получает `lv_display_get_default()`, выставляет rotation при необходимости;
   - `process()` делает `lv_tick_inc(delta)`, `lv_timer_handler()`, `ui_tick()`;
   - `setBrightness(float)` прокидывает в `smartdisplay_lcd_set_backlight`.
4. Создать точку инициализации UI runtime в `App::init()` до `State::init()`.
5. Временно оставить старый Nextion boot, но завести feature flag или compile-time switch для нового UI boot path.
6. Реализовать мост для EEZ native actions:
   - `action_keyboard_text(lv_event_t * e)`;
   - `action_keyboard_number(lv_event_t * e)`;
   - на первом шаге допустимы заглушки в `Screen/Panel`, позже они должны открыть общий экран `Keyboard` в нужном режиме.

## Проверки

- `pio run -e JC8048W550C` проходит.
- В dependency graph появились `lvgl`, `eez_ui` и `esp32_smartdisplay`.
- На устройстве виден экран `Load0`, touch не ломает loop.
- Boot не зависает при отсутствии UART Nextion.

## Риски

- `lv_conf.h` лежит в стандартном `include/lv_conf.h`; если PlatformIO/LVGL не найдут его автоматически, нужен явный `LV_CONF_PATH`.
- `lib/eez_ui/src/ui/actions.h` объявляет keyboard actions; без реализаций линковка упадет после подключения `eez_ui`.
