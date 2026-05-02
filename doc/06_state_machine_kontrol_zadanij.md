# Этап 6. State и Scene как контроль заданий

## Задача этапа

Сохранить state machine, но перевести ее на работу с отдельным модулем сцен.

Состояния не должны напрямую формировать CAN-команды. Они должны вызывать тематическую сцену, а сцена уже решает, как отправить задание одному или нескольким device.

## Новый паттерн State

Структура остается прежней:

```cpp
void oneRun() override {
    State::oneRun();
    taskId = App::scene().paper().feed(...);
}

State* run() override {
    SceneTaskStatus status = App::scene().status(taskId);

    if (status.running()) return this;
    if (status.done()) return Factory(State::Type::DONE);
    if (status.failed()) return Factory(State::Type::ERROR);

    return this;
}
```

Главное изменение: `oneRun()` больше не запускает локальный мотор. Он вызывает нужную сцену, а сцена отправляет task конкретному device или группе device.

## Модуль Scene

Новая папка:

```text
src/Scene/
  Base.h/.cpp
  SceneManager.h/.cpp
  SceneTask.h
  PaperScene.h/.cpp
  GuillotineScene.h/.cpp
  TableScene.h/.cpp
  CheckScene.h/.cpp
  EmergencyScene.h/.cpp
```

Так сцены не смешиваются в один большой класс.

## BaseScene

`BaseScene` содержит общее:

- ссылку на `DeviceRegistry`;
- доступ к mapping из config;
- отправку `DeviceTask`;
- создание `SceneTask`;
- агрегацию статусов;
- helpers для timeout/offline/error.

Пример:

```cpp
class BaseScene {
protected:
    SceneTaskId sendToRole(Role role, DeviceCommand cmd, const DeviceParams& p, uint32_t timeoutMs);
    SceneTaskId sendToRequired(DeviceCommand cmd, const DeviceParams& p, uint32_t timeoutMs);
    SceneTaskStatus aggregate(SceneTaskId taskId) const;
};
```

`Role` и `DeviceCommand` - enum, `DeviceParams` - POD-union (см. этап 3 и 4). В сценах нет `String` и `JsonDocument`.

## Тематические сцены

`PaperScene`:

- `feed(...)`;
- `feedUntilMark(...)`;
- `feedUntilEdge(...)`;
- `profileFeed(...)`;
- `stop()`.

`GuillotineScene`:

- `cut()`;
- `forward()`;
- `backward()`;
- `checkReady()`;
- `stop()`.

`TableScene`:

- `up()`;
- `down()`;
- `checkPosition()`;
- `stop()`.

`CheckScene`:

- `all()`;
- `requiredDevices()`;
- `device(name)`;
- сбор результатов self-test.

`EmergencyScene`:

- `stopAll()`;
- `safeState()`;
- `resetErrors()`, если разрешено;
- broadcast-команды.

## SceneManager

`SceneManager` - единая точка входа для `State`:

```cpp
class SceneManager {
public:
    PaperScene& paper();
    GuillotineScene& guillotine();
    TableScene& table();
    CheckScene& check();
    EmergencyScene& emergency();

    SceneTaskStatus status(SceneTaskId id) const;
};
```

Можно временно сохранить старые методы для совместимости:

```cpp
SceneTaskId SceneManager::paperMove(...) {
    return paper().feed(...);
}
```

## Пример Check

Раньше `Check` мог последовательно проверять конкретные моторы и датчики.

Теперь:

```text
Check::oneRun()
  App::scene().check().all()
    -> send "check" to all required device

Check::run()
  -> collect statuses
  -> if all done ok: DONE/IDLE
  -> if any failed: ERROR
  -> if timeout/offline: ERROR
```

Такое состояние становится проще: оно не делает механику, а собирает результат самопроверки соседей.

## Пример подачи бумаги

```text
PaperMove::oneRun()
  App::scene().paper().feed(mm/steps/profile params)
    -> target device from config scene.paper
    -> command "paper.feed"

PaperMove::run()
  -> wait task done/failed/timeout
```

Если движение требует датчика метки, это должно быть заданием device:

- `paper.feed_until_mark`;
- `paper.feed_until_edge`;
- `paper.feed_profile`.

Не надо опрашивать датчик с головного ESP32 через CAN на каждом цикле.

## Пример гильотины

```text
GuillotineForward::oneRun()
  App::scene().guillotine().cut()
    -> target device from config scene.guillotine или scene.motion
    -> command "guillotine.cut"

GuillotineForward::run()
  -> wait done/failed/timeout
```

Внутри device может быть мотор, датчик положения, концевик, таймауты. Головное устройство получает только результат задания.

## Экстренная остановка

Экстренная остановка должна быть отдельной сценой, а не кусками в разных местах.

```text
EmergencyScene::stopAll()
  -> broadcast "stop"
  -> stop required devices
  -> wait short ack window
  -> mark devices without ack as unsafe/offline
```

Эта сцена должна быть доступна из любого состояния и из UI.

## Статусы заданий

Нужен общий task tracker:

```cpp
struct SceneTask {
    uint32_t id;
    String command;
    std::vector<DeviceTaskId> deviceTasks;
    uint32_t startedAtMs;
    uint32_t timeoutMs;
};
```

`BaseScene` или `SceneManager` агрегирует статусы нескольких device:

- все `Done` -> scene task `Done`;
- хотя бы один `Failed` -> `Failed`;
- required device offline -> `Failed`;
- timeout -> `Timeout`;
- часть еще `Running` -> `Running`.

## Ошибки

Нужны сетевые ошибки, но можно добавлять постепенно:

- device offline;
- task timeout;
- task rejected;
- device error;
- protocol error;
- CAN bus error.

На первом этапе можно маппить их в существующие `Catalog::ErrorCode`, но в `DeviceError.value` писать точный текст.

## Порядок миграции состояний

1. `BOOT`: поднять CAN, настроить device, ждать ready.
2. `CHECK`: использовать `App::scene().check().all()`.
3. Простые движения: `PAPER_MOVE` через `PaperScene`, `TABLE_UP` через `TableScene`, `GUILLOTINE_FORWARD` через `GuillotineScene`.
4. Состояния с поиском: `DETECT_PAPER`, `DETECT_MARK`, но как крупные задания `PaperScene`.
5. Большие сценарии: `PROCESS`, `CALIBRATION`, `PROFILING`, `SLICE`.
6. Экстренную остановку перевести на `EmergencyScene` как общий механизм.

## Критерий готовности

- Есть отдельная папка `src/Scene`.
- Есть `BaseScene` и `SceneManager`.
- Сцены разделены по доменам: бумага, гильотина, стол, check, emergency.
- `Scene` умеет отправить task одному device.
- `Scene` умеет отправить task группе required device.
- Состояние `CHECK` работает через `CheckScene`.
- Хотя бы одно состояние движения работает через тематическую сцену и CAN-device.
- `State::oneRun/run` сохранены.

