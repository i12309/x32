# План разделения `Service/CAN`

## Цель

Разделить текущий `src/Service/CAN.*` по ответственностям, сохранив публичный API `CAN::instance()` для остального кода.

## Новая структура

```text
src/Service/CAN.h              // совместимый include-фасад
src/Service/CAN/
  CAN.h                        // публичный фасад CAN
  CAN.cpp
  CanBootDiscovery.h
  CanBootDiscovery.cpp
  CanBusRuntime.h
  CanBusRuntime.cpp
  CanAddressBook.h
  CanAddressBook.cpp
  CanScenarioService.h
  CanScenarioService.cpp
```

## Разделение

### 1. `CanBus`

- Старт TWAI/CAN после загрузки `config.json`.
- Хранит `Esp32TwaiBus`, `CanNetwork`, `ready`.
- Методы: `begin()`, `isReady()`, `bus()`, `network()`, `lastError()`.

### 2. `CanBoot`

- BOOT discovery через `Mgmt`.
- Ждет `BootHello`.
- Ищет ноду по MAC.
- Назначает CAN ID.
- Ждет `AssignAck`.
- Формирует ошибки discovery.

### 3. `CanHelper`

- Читает адреса из `Core::config`.
- Методы: `nodeAddress()`, `nodeGroup()`, `feedGroup()`.
- Поместить сюда все методы которые можно будет переиспользовать в других классах. Это все вспомогательные методы поиска и преобразования

### 4. `CanScenario`

- Runtime-команды станка через `Scenario::Client`.
- `checkAll()`, `stopAll()`.
- `table*`, `guillotine*`, `paper*`, `detect*`, `throwRun()`.
- Внутренние: `runScenario()`, `startScenario()`, `waitScenario()`, `mmToSteps()`.

### 5. `CAN` фасад

- Оставляет старые публичные методы.
- Делегирует работу в `CanBus`, `CanBoot`, `CanScenario`.
- Нужен, чтобы не менять вызовы вида `CAN::instance().paperMoveSteps(...)`.

## Порядок работ

1. Создать папку `src/Service/CAN/`.
2. Перенести `src/Service/CAN.h/.cpp` в `src/Service/CAN/CAN.h/.cpp`.
3. Оставить `src/Service/CAN.h` как совместимый include:

```cpp
#pragma once
#include "Service/CAN/CAN.h"
```

4. Вынести `CanBoot`.
5. Вынести `CanBus`.
6. Вынести `CanHelper`.
7. Вынести `CanScenario`.
8. После каждого шага запускать `pio run`.

## Ограничения

- Не менять внешний API класса `CAN` на первом этапе.
- Не менять протокол `Scenario`.
- Не добавлять `CAN.enabled`.
- Не переносить логику config payload, это этап 4 BOOT-плана.
