# Этап 01: запуск LVGL/EEZ на JC8048W550C

## Цель

Получить минимально работающий экран на `JC8048W550C`:

- физический дисплей и touch инициализируются через `lib/esp32-smartdisplay`;
- LVGL получает tick и регулярно обрабатывает отрисовку/input;
- EEZ generated UI из `lib/eez_ui` создает экраны и показывает `Load0` .

Важно: на этом этапе не переносим бизнес-логику страниц. Мы только включаем runtime, чтобы дальше классы из `src/Screen/Page` могли работать поверх готовых EEZ-объектов.

## Что такое bootstrap в этом плане

`bootstrap` здесь означает не отдельный UI-фреймворк, а минимальный код запуска экрана:

```cpp
smartdisplay_init(); // физический экран + touch -> LVGL
ui_init();           // EEZ создает screens/objects
```

и в основном цикле:

```cpp
lv_tick_inc(delta);
lv_timer_handler();
ui_tick();
```

Generated EEZ C-code умеет создавать `lv_obj_t` и переключать экраны, но сам не инициализирует железо, touch, backlight и LVGL timer loop. Поэтому небольшой runtime-класс в `src/Screen/Panel` все равно нужен.

## Текущая структура библиотек

- `lib/lvgl` - LVGL 9.4.0 как локальная PlatformIO-библиотека.
- `lib/lvgl/lvgl/lvgl.h` - compatibility header для generated EEZ include `#include <lvgl/lvgl.h>`.
- `lib/eez_ui` - EEZ-проект и generated C-code.
- `lib/eez_ui/library.json` - описание локальной библиотеки EEZ UI.
- `lib/esp32-smartdisplay` - драйвер дисплея/touch.
- `include/lv_conf.h` - конфиг LVGL проекта.

## Работы

1. Проверить конфигурацию PlatformIO:
   - после подключения runtime-кода в dependency graph должны появиться `lvgl`, `eez_ui` и `esp32_smartdisplay`;
   - `lib_extra_dirs` для старого `include/ui/vendor` больше не нужен;
   - `LV_CONF_PATH` можно не добавлять, пока `include/lv_conf.h` находится стандартным include path.

2. Создать runtime-класс экрана в `src/Screen/Panel`, например `Panel.h|cpp`:
   - `init()` вызывает `smartdisplay_init()`;
   - после инициализации драйвера вызывает `ui_init()`;
   - при необходимости выставляет rotation/backlight;
   - `process()` вызывает `lv_tick_inc(delta)`, `lv_timer_handler()`, `ui_tick()`.

3. Подключить runtime к жизненному циклу приложения:
   - вызвать `Panel::init()` из `App::init()` до старта UI-зависимых state flow;
   - вызвать `Panel::process()` из `App::process()`.

4. Временно оставить старый Nextion boot path:
   - не удалять `src/UI` и `lib/Nextion` на этом этапе;
   - добавить compile-time switch или аккуратную точку выбора, чтобы можно было перейти на LVGL boot без полного удаления старого UI.

5. Реализовать EEZ native actions:
   - `action_keyboard_text(lv_event_t * e)`;
   - `action_keyboard_number(lv_event_t * e)`.

   На первом шаге допустимы заглушки, чтобы прошла линковка после подключения `eez_ui`. Позже эти функции станут мостом к экрану `Keyboard`.

## Проверки

- `pio run -e JC8048W550C` проходит.
- В dependency graph появились `lvgl`, `eez_ui` и `esp32_smartdisplay`.
- На устройстве виден `Load0`.
- Touch не ломает loop.
- Boot не зависает при отсутствии UART Nextion, если включен LVGL boot path.

## Риски

- Без реализации `action_keyboard_text` и `action_keyboard_number` линковка упадет, потому что `screens.c` уже навешивает эти callbacks.
- Если LVGL не найдет `include/lv_conf.h` автоматически, нужно добавить явный build flag:

```ini
build_flags =
    -I src
    -D LV_CONF_PATH=${PROJECT_INCLUDE_DIR}/lv_conf.h
```

- Текущая успешная сборка пока проверяет старый Nextion UI. Новый стек будет реально проверен только после появления кода, который включает `esp32_smartdisplay.h` и `ui/ui.h`.

## Граница этапа

Этап считается завершенным, когда приложение может показать EEZ-экран на физическом дисплее через LVGL, но бизнес-логика страниц еще может оставаться старой или пустой.
