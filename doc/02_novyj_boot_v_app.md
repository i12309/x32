# Этап 2. BOOT остается состоянием

## Важное изменение плана

`Boot.h` не надо переносить в `App`.

BOOT должен остаться состоянием state machine, потому что новая распределенная архитектура хорошо ложится на паттерн:

- `oneRun()` - задать последовательность boot-задач и отправить первичные задания device;
- `run()` - ждать завершения шагов, статусов device и переходить дальше.

Название файла осталось историческим, но актуальный смысл этого этапа: переделать текущий `State/Device/Boot.h`, а не выносить его в `src/App`.

## Новая роль BOOT

BOOT остается первым состоянием:

```cpp
void State::init() {
    App::state() = new Boot();
    App::state()->oneRun();
}
```

BOOT должен:

- поднять базовые сервисы;
- поднять `Screen/Panel`;
- поднять CAN;
- загрузить config;
- создать реестр соседних CAN-device;
- отправить device настройки;
- дождаться статусов `ready` от обязательных device;
- загрузить данные;
- перейти в `CHECK` или `IDLE`.

## Что меняется в oneRun

Старый `oneRun()` сейчас добавляет набор `ACTION`.

Это можно сохранить. Но действия должны стать другими:

```text
LogStart
InitBoard
InitDisplay
ShowLoad
InitFileSystem
InitNVS
LoadConfig
InitCanBus
InitDeviceRegistry
ConfigureDevices
WaitDevicesReady
ConnectWiFi
UpdateFirmware
StartHttp
ConnectMQTT
LoadData
RegisterRuntimeEvents
```

Если часть шагов асинхронная, она может быть отдельным состоянием или action, который только стартует работу, а `run()` BOOT потом ждет результат через `Scene`/`DeviceRegistry`.

## Что меняется в run

`run()` должен остаться местом принятия решения:

- boot halted;
- boot failed;
- device offline;
- обязательные device ready;
- идти в `CHECK`;
- идти в `IDLE`;
- остаться в BOOT и ждать.

Пример смысла:

```text
if waiting for required devices:
  if all ready -> continue plan
  if timeout -> ERROR or NULL_STATE
  else -> return this
```

## BOOT и CAN-device

BOOT не должен знать внутренности device.

Правильные boot-команды:

- `configure` - отправить настройки из config;
- `self_test` - попросить device выполнить самопроверку;
- `status` - запросить состояние;
- `stop` - привести device в безопасное состояние;
- `reset_error` - сбросить ошибки, если это разрешено.

Неправильно:

- проверять пины датчиков с головного ESP32;
- знать, сколько моторов внутри STM32-платы;
- опрашивать энкодер как отдельную сущность, если это внутренняя часть device.

## BOOT и Screen

BOOT должен работать с экраном через `Screen/Panel` и страницы из `Screen/Page`.

Правильная граница:

```text
Boot
  -> Screen::Panel::init()
  -> Screen::load::show()
  -> load::text(...)
```

BOOT не должен напрямую знать LVGL objects, EEZ generated переменные или драйвер touch.

## Что убрать из BOOT

- `InitNextion`;
- `TryRecoverNextionIfMissing`;
- `SetTFTVersion`;
- `UpdateTFT` в виде загрузки `.tft`;
- прямую регистрацию `McpTrigger`;
- создание локальных `MCP`, `FastAccelStepper`, GPIO-датчиков для головного режима.

## MachineSpec в BOOT

Пока не делать жесткую проверку `MachineSpec`.

На первом этапе BOOT должен проверять только:

- config прочитан;
- CAN поднят;
- required device из config есть или корректно отмечены offline;
- настройки отправлены;
- получен понятный статус.

## Критерий готовности

- `State/Device/Boot.h` остается точкой входа state machine.
- BOOT больше не инициализирует Nextion и локальное железо.
- BOOT использует `Screen/Panel` для загрузки экрана и `Screen::load` для статусов.
- BOOT умеет поднять CAN и создать `DeviceRegistry`.
- BOOT может ждать ready/error/offline от device.
- После BOOT переход идет в `CHECK` или `IDLE`, как и раньше.
