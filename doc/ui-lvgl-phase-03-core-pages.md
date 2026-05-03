# Этап 03: перенести базовые страницы запуска и навигации

## Цель

Перенести минимальный пользовательский путь: загрузка, главное меню, информационные окна, ввод, ожидание.

## Страницы

- `Load0`
- `Load`
- `Main`
- `Info`
- `Input`
- `INIT`
- `Wait`
- `Keyboard`

## Работы

1. `Load`:
   - перенести `pLoad::text`, progress labels, mac/version display;
   - заменить `getHMIVersion()` на firmware/UI version из кода, потому что у LVGL нет отдельной Nextion HMI version по UART;
   - переписать `Boot::InitNextion`, `ShowLoad`, `SetTFTVersion`, `UpdateTFT` в LVGL-терминах.
2. `Main`:
   - перенести callbacks главных кнопок: task, profile, wifi/settings, service, stats, help/update;
   - использовать `PageManager` для переходов.
3. `Info`:
   - сделать API `showInfo(title, field1, field2, onOk, onCancel, modalFlags...)`;
   - заменить старые `pINFO::showInfo(...)` вызовы по проекту на новый фасад.
4. `Input` и `Keyboard`:
   - реализовать ввод через LVGL textarea/keyboard;
   - использовать generated actions `action_keyboard_text` и `action_keyboard_number` только как мост к C++ input service.
5. `INIT`:
   - перенести выбор машины, HTTP, access point, test flags.

## Проверки

- Boot доходит до `Main`.
- Ошибки boot показываются через новый `Info`.
- Тексты версии и MAC отображаются на `Load`.
- Кнопки `Main` открывают нужные страницы.
- Keyboard вводит текст/числа без UART и без Nextion-команд.

## Риски

- В старом boot есть логика восстановления/обновления Nextion `.tft`; для нового экрана она должна быть удалена или заменена на проверку версии generated UI.
- Много сервисов напрямую включают `UI/Main/pINFO.h`, `pINIT.h`, `pLoad.h`; на этом этапе нужен совместимый facade, чтобы не ломать весь проект сразу.
