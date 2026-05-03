# План рефакторинга UI: Nextion -> LVGL/EEZ/JC8048W550C

## Цель

Перевести UI с Nextion UART на локальную отрисовку LVGL на экране `JC8048W550C` через `lib/esp32-smartdisplay`.

Сгенерированный EEZ код остается нижним слоем и не содержит бизнес-логики:

- источник интерфейса: `include/ui/eez_project/SMIT_v1.eez-project`;
- generated LVGL C code: `include/ui/eez_project/src/ui`;
- LVGL 9.4.0: `include/ui/vendor/lvgl-release-v9.4`;
- новая C++ логика страниц: `src/Screen/Page`;
- экранные/графические runtime-классы: `src/Screen/Panel`.

## Текущая картина

- Текущая сборка `pio run -e JC8048W550C` проходит, но собирает старый Nextion UI.
- В dependency graph текущей сборки нет `lvgl` и `esp32_smartdisplay`, потому что новый слой еще не подключен из кода.
- EEZ-проект содержит 26 страниц 800x480: `Load0`, `Load`, `Main`, `TaskRun`, `TaskProcess`, `Task`, `Profile`, `List`, `Info`, `Input`, `INIT`, `Wait`, `Service`, `Table`, `Paper`, `Guillotine`, `Service2`, `Throws`, `Bigel`, `Wifi`, `Keyboard`, `Stats`, `Update`, `Calibration`, `Slice`, `Page`.
- `flowSupport=false`, поэтому переходы, callbacks и данные должны жить в C++ слое поверх `objects`.
- Старые `src/UI/**` страницы смешивают Nextion-объекты, показ страниц, бизнес-логику и переходы. Их нужно переносить постепенно.

## Архитектурное решение

1. `Screen/Panel` отвечает за инициализацию дисплея, LVGL tick/handler, lifecycle generated UI и тонкие обертки над `lv_obj_t`.
2. `Screen/Page` отвечает за lifecycle страниц: `onPrepare()`, `onShow()`, `onHide()`, `onTick()`.
3. Классы страниц не создают LVGL-объекты и не настраивают layout/style. Они только:
   - читают/пишут значения через обертки;
   - назначают обработчики событий;
   - вызывают бизнес-сервисы, state machine, навигацию.
4. Сгенерированные файлы `include/ui/eez_project/src/ui/*` не редактируются руками. Если нужно менять вид, меняется `.eez-project` и выполняется regenerate.
5. Старый `src/UI` удаляется только после полного переноса зависимостей из `State`, `Service`, `App` и страниц.

## Этапы

- Этап 01: `doc/ui-lvgl-phase-01-display-bootstrap.md`
- Этап 02: `doc/ui-lvgl-phase-02-runtime-and-page-base.md`
- Этап 03: `doc/ui-lvgl-phase-03-core-pages.md`
- Этап 04: `doc/ui-lvgl-phase-04-domain-pages.md`
- Этап 05: `doc/ui-lvgl-phase-05-nextion-removal.md`

## Правило миграции страницы

Для каждой страницы перенос идет одинаково:

1. Найти старый класс `src/UI/**/p*.h|cpp`.
2. Найти generated объекты в `include/ui/eez_project/src/ui/screens.h`.
3. Создать класс в `src/Screen/Page/<Name>.h|cpp`.
4. В `onPrepare()` связать элементы, значения и callbacks.
5. В `onShow()` заполнить состояние экрана.
6. В `onTick()` оставить только периодическую бизнес-логику.
7. Заменить переходы старого класса на `Screen::pages().show("<Name>")` или typed API.
8. Собрать и проверить сценарий на устройстве.

## Критерий завершения

- Приложение запускает `smartdisplay_init()`, `ui_init()`, `lv_tick_inc()`, `lv_timer_handler()` и `ui_tick()`.
- Все переходы работают без `nexLoop`, `sendCommand`, `NexText`, `NexButton`, `NexPage`.
- `lib/Nextion`, `Service/NexUpdate*`, `NexUploadHTTP*` и Nextion-ветки boot/update удалены или изолированы как legacy.
- Сборка `pio run -e JC8048W550C` проходит с LVGL/EEZ UI.
