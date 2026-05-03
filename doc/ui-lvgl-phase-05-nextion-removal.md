# Этап 05: удалить Nextion-зависимости и legacy boot/update

## Цель

После переноса всех страниц убрать Nextion как runtime dependency и оставить только LVGL/EEZ/SmartDisplay UI.

## Работы

1. Удалить или изолировать:
   - `lib/Nextion`;
   - `src/UI`;
   - `src/Service/NexUpdate.h`;
   - `src/Service/NexUploadHTTP.h`;
   - Nextion-specific ветки в `ESPUpdate`, `Boot`, `version`.
2. Очистить includes:
   - `<Nextion.h>`;
   - `"NexHardware.h"`;
   - `NexText`, `NexButton`, `NexPage`, `NexTouch`, `sendCommand`, `recvRetNumber`, `nexLoop`.
3. Обновить `App::Context::UiContext`:
   - заменить `Page** activePage/previousPage` старого типа на новый `IPage**` или API `PageManager`.
4. Обновить update policy:
   - прошивка ESP остается через текущий OTA;
   - обновление `.tft` больше не нужно;
   - version generated UI хранится в коде или отдельном metadata-файле.
5. Очистить `platformio.ini` и локальные библиотеки от неиспользуемых зависимостей.

## Проверки

- `rg "Nextion|NexText|NexButton|NexPage|NexTouch|sendCommand|nexLoop|NexUpdate|NexUpload"` не находит runtime-зависимостей.
- `pio run -e JC8048W550C` проходит.
- Boot, error flow, main navigation, task run, service screens и update screen проверены на устройстве.
- Flash/RAM usage зафиксированы после удаления legacy.

## Риски

- Старые сервисы обновления могли использовать Nextion version как часть общей версии устройства. Это нужно заменить до удаления.
- Если где-то физическая кнопка была привязана к Nextion callback path, ее нужно перенести в `Trigger`/`Registry` path.
