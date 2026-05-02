# Этап 1. Архитектура головного устройства

## Задача этапа

Зафиксировать новую архитектуру без уничтожения текущей state machine.

Основная мысль: головное устройство управляет не пинами и моторами, а соседними device на CAN. `State` вызывает тематические сцены, а сцены превращают верхний сценарий в задания для device.

## Новая структура модулей

```text
src/App/
  App.h/.cpp

src/State/
  State.h/.cpp
  Device/Boot.h
  ...

src/Scene/
  Base.h/.cpp
  SceneManager.h/.cpp
  SceneTask.h
  PaperScene.h/.cpp
  GuillotineScene.h/.cpp
  TableScene.h/.cpp
  CheckScene.h/.cpp
  EmergencyScene.h/.cpp

src/Bus/
  ICanBus.h
  Esp32TwaiBus.h/.cpp
  CanFrame.h

src/Can/
  MksCan.h/.cpp
  StmCan.h/.cpp
  CanRouter.h/.cpp

src/Device/
  DeviceRegistry.h/.cpp
  DeviceNode.h
  DeviceTask.h
  DeviceStatus.h

src/Screen/
  Panel/
    Panel.h/.cpp
    EezPanel.h/.cpp
    DisplayDriver.h/.cpp
    TouchDriver.h/.cpp
    Model/
      ScreenModel.h
      MainModel.h
      TaskModel.h
      ServiceModel.h
      ...
  Page/
    Page.h/.cpp
    Main/
      load.h/.cpp
      main.h/.cpp
      info.h/.cpp
      error.h/.cpp
      ...
    Task/
      task.h/.cpp
      task_run.h/.cpp
      ...
    Service/
      service.h/.cpp
      guillotine.h/.cpp
      paper.h/.cpp
      ...
```

## CAN-слой

Иерархия:

```text
Esp32TwaiBus : ICanBus
        |
        +-- MksCan
        |     общение с MKS Servo57D_CAN
        |
        +-- StmCan
              общение с будущими STM32-платами
```

`Esp32TwaiBus` отвечает только за физическую отправку и прием CAN-кадров через TWAI.

`MksCan` и `StmCan` знают форматы команд и ответов конкретного семейства устройств.

`CanRouter` принимает кадры, определяет адрес/device/protocol и обновляет `DeviceRegistry`.

## DeviceRegistry

`DeviceRegistry` должен быть абстрактным реестром соседей:

- `name`;
- `address`;
- `type`;
- `protocol`;
- `required`;
- `online/offline`;
- `ready/busy/error`;
- настройки из config;
- активные задания и их статусы.

Не надо на этом уровне делать отдельные `MotorProxy`, `SensorProxy`, `EncoderProxy`. Это преждевременная детализация. Для головного устройства важнее уметь отправить device задание и получить состояние.

## Scene

`Scene` лучше вынести из `src/State/Scene.*` в отдельный модуль `src/Scene`.

Причина: после перехода на CAN сцены станут главным слоем оркестрации заданий. Если оставить все в одном классе, он быстро превратится в смесь подачи бумаги, гильотины, проверок, аварийной остановки и сервисных команд.

Нужна структура:

```text
BaseScene
  общие зависимости и helper-методы:
  DeviceRegistry;
  scene mapping из config;
  отправка task;
  сбор статусов;
  timeout/error helpers.

PaperScene
  подача бумаги;
  поиск метки;
  протяжка на расстояние;
  профильная подача.

GuillotineScene
  рез;
  движение гильотины;
  проверка готовности гильотины.

TableScene
  подъем/опускание стола;
  проверка положения.

CheckScene
  общая проверка device;
  сбор self-test статусов.

EmergencyScene
  broadcast stop;
  safe state;
  reset errors, если разрешено.

SceneManager
  единая точка доступа для State:
  scene.paper().feed(...);
  scene.guillotine().cut(...);
  scene.check().all();
  scene.emergency().stopAll();
```

## BaseScene

`BaseScene` не должен описывать конкретную механику. Он должен дать общие инструменты:

- найти device по роли из config;
- отправить `DeviceTask`;
- отправить task нескольким device;
- создать `SceneTask`;
- проверить агрегированный статус;
- сформировать ошибку для `DeviceError`.

Пример смысла:

```cpp
class BaseScene {
protected:
    DeviceTaskId sendToRole(const String& role, const String& command, JsonObjectConst params);
    SceneTaskId trackOne(DeviceTaskId task);
    SceneTaskStatus status(SceneTaskId taskId) const;
};
```

## SceneManager

`App::scene()` может вернуть не один огромный `Scene`, а менеджер:

```cpp
App::scene().paper().feed(...);
App::scene().guillotine().cut(...);
App::scene().check().all();
App::scene().emergency().stopAll();
```

Для совместимости можно временно оставить методы старого `Scene` и внутри прокинуть их в новые классы:

```cpp
SceneTaskId SceneManager::paperMove(...) {
    return paperScene.feed(...);
}
```

## Screen

Экранный код лучше вынести в новую папку `src/Screen`, а старую `src/UI` постепенно оставить как legacy или удалить после переноса.

Внутри `Screen` две зоны ответственности:

```text
src/Screen/Page/
  бизнес-логика страниц.
  Группировка папок сохраняется близкой к старой src/UI:
  Main, Task, Service, Wifi, Statistics, Profile, Help ...
  Префикс p убирается:
  pLoad -> load, pMain -> main, pINFO -> info.

src/Screen/Panel/
  управление экраном:
  init дисплея;
  init touch;
  init LVGL;
  init EEZ generated UI;
  общий process/tick;
  показ страниц по id;
  объектная модель страниц и элементов;
  обновление текста, цветов, visible/value без id в page-классах.
```

`Page` не должен знать детали дисплейного драйвера и LVGL init. Он вызывает методы `Panel`.

`Page` не должен знать числовые id элементов. Он работает с именованной объектной моделью:

```cpp
model().main.load.statusText.setText("...");
model().main.load.progress[0].setColor(color);
```

`Panel` не должен содержать бизнес-логику станка. Он обслуживает экран, держит объектную модель, скрывает EEZ/generated ids и доставляет события страницам.

## State

Состояния сохраняют текущий паттерн:

```cpp
void oneRun() override {
    // отправить задание через нужную Scene
}

State* run() override {
    // ждать scene/device status
}
```

Состояния станут проще, потому что сложная realtime-логика уйдет внутрь конечных device.

## MachineSpec

На первом этапе `MachineSpec` лучше сделать пустым или неблокирующим.

Причина: структура device и scene mapping еще будут меняться. Раннее жесткое описание требований может начать мешать архитектурной миграции.

Позже `MachineSpec` можно вернуть как проверку сценариев:

- какие device нужны для типа станка;
- какие задания они должны поддерживать;
- какие статусы обязательны.

## Критерий готовности

- В проекте есть интерфейсы `ICanBus`, `DeviceRegistry`, `DeviceTask`, `DeviceStatus`.
- Есть план отдельного модуля `src/Scene`.
- Есть `BaseScene` и тематические сцены: бумага, гильотина, check, emergency.
- `State` получает доступ к сценам через `App::scene()` или `SceneManager`.
- В плане UI используется новая структура `src/Screen/Page` и `src/Screen/Panel`.
- `Screen/Page` сохраняет группировку старых UI-папок, но классы страниц называются без префикса `p`.
- В `Screen/Page` нет обращений к id элементов, только к именованной модели.
- Старый код еще может собираться.
- Нет требования переносить `BOOT` из `State`.
- `MachineSpec` не блокирует boot на этапе архитектурной миграции.
