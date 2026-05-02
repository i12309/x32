# Этап 3. CAN-транспорт и протоколы

## Задача этапа

Добавить CAN-слой, который позволит головному ESP32 общаться с разными семействами устройств:

- MKS Servo57D_CAN;
- будущие STM32-платы;
- возможно, другие устройства позже.

## Разделение ответственности

Шина и протокол - это разные слои. Шина одна (физическая TWAI), протоколов на ней может быть несколько (MKS, STM32, в будущем другие). Поэтому `MksCan` и `StmCan` не наследуются от `Esp32TwaiBus`, а получают ссылку на `ICanBus` в конструкторе:

```text
ICanBus
  низкоуровневая отправка/прием CAN-кадров

Esp32TwaiBus : ICanBus
  реализация через TWAI ESP32-S3

MksCan
  протокол MKS Servo57D_CAN
  держит ICanBus*, отправляет/принимает кадры через него

StmCan
  протокол STM32-плат
  держит ICanBus*, отправляет/принимает кадры через него

CanRouter
  принимает входящие кадры от ICanBus,
  по CAN id определяет адрес device и его протокол,
  передает кадр в нужный экземпляр MksCan/StmCan,
  обновляет DeviceRegistry
```

Сборка выглядит так:

```cpp
Esp32TwaiBus bus;
bus.begin(canCfg);

MksCan mks(bus);
StmCan stm(bus);

CanRouter router(bus, registry);
router.bindProtocol("mks",    &mks);
router.bindProtocol("stm.v1", &stm);
```

На одной физической шине одновременно работают и MKS-устройства, и STM32-устройства. Если в будущем появится вторая шина, добавляется второй `Esp32TwaiBus` и второй `CanRouter`, а классы протоколов остаются прежними.

`Esp32TwaiBus` не должен знать, что такое `move`, `check`, `ready`. Он знает только CAN id, payload, ошибки шины.

`MksCan` и `StmCan` знают команды и ответы своих устройств, но не знают, как именно отправляется кадр в железо - это делает `ICanBus`.

## ICanBus

Минимальный интерфейс:

```cpp
class ICanBus {
public:
    virtual bool begin(const CanBusConfig& cfg) = 0;
    virtual bool send(const CanFrame& frame) = 0;
    virtual bool receive(CanFrame& frame) = 0;
    virtual void process() = 0;
    virtual CanBusStatus status() const = 0;
};
```

## DeviceTask

Вместо детальных `MotorProxy/SensorProxy` нужен общий тип задания. Важное требование: `DeviceTask` создается часто (на каждое движение, check, heartbeat-poll), поэтому в нем не должно быть динамических аллокаций. Никаких `String` и `JsonDocument` внутри `DeviceTask`.

Целевой вид:

```cpp
enum class DeviceCommand : uint8_t {
    Configure,
    SelfTest,
    Check,
    Stop,
    ResetError,
    PaperFeed,
    PaperFeedUntilMark,
    TableUp,
    TableDown,
    GuillotineCut,
    ProfileRun
};

struct DeviceTask {
    uint32_t       id;
    uint8_t        deviceAddress;   // CAN-адрес, не имя
    DeviceCommand  command;
    DeviceParams   params;          // POD-union с параметрами под команду
    uint32_t       timeoutMs;
};
```

Параметры конкретных команд - в union, без heap:

```cpp
struct DeviceParams {
    union {
        struct { float mm; uint16_t speedMmS; } paperFeed;
        struct { uint8_t profileId; }            profileRun;
        struct { uint16_t cutSpeedMmS; }         guillotineCut;
        // configure не помещается в union - см. ниже
    } u;
};
```

`configure` - исключение: его payload большой и читается из конфига. Для него `DeviceTask` хранит не сам JSON, а `JsonObjectConst` (ссылка на узел в уже распарсенном корневом `JsonDocument` конфига - это просто пара указателей, без копирования):

```cpp
struct ConfigureTask {
    uint32_t        id;
    uint8_t         deviceAddress;
    JsonObjectConst payload;        // ссылка в Core::config, живет столько же сколько config
    uint32_t        timeoutMs;
};
```

Адресация по `uint8_t deviceAddress`, а не по строке. Маппинг "имя из конфига → адрес" разрешается один раз при boot в `DeviceRegistry`. На уровне `Scene` можно использовать `enum class Role` (см. п. про роль/имя ниже), а `DeviceRegistry::sendTask(role, ...)` сам подставит адрес.

Примеры команд (значения `DeviceCommand`):

- `Configure`;
- `SelfTest`;
- `Check`;
- `Stop`;
- `ResetError`;
- `PaperFeed`;
- `PaperFeedUntilMark`;
- `TableUp`;
- `TableDown`;
- `GuillotineCut`;
- `ProfileRun`.

Команды верхнего уровня должны описывать намерение. Конечный device сам решает, какие моторы, датчики и энкодеры использовать внутри.

Если на каком-то этапе потребуется команда с параметрами, не влезающими в union - не расширять union бесконечно, а добавить отдельный struct (`ConfigureTask`-стиль) и отдельную ветку отправки.

## DeviceStatus

```cpp
enum class DeviceTaskStatus {
    Unknown,
    Queued,
    Sent,
    Accepted,
    Running,
    Done,
    Failed,
    Timeout,
    Rejected
};

struct DeviceStatus {
    String deviceName;
    bool online;
    bool ready;
    bool busy;
    bool error;
    String errorText;
};
```

## MksCan

`MksCan` нужен для общения с MKS Servo57D_CAN.

Он может поддерживать только те команды, которые реально есть у MKS:

- enable/disable;
- move;
- stop;
- read position/status;
- clear error.

Если верхняя `Scene` отправит команду, которую MKS не поддерживает напрямую, `MksCan` должен либо разложить ее на поддерживаемые команды, либо вернуть `Rejected`.

## Capability набора команд

Каждый протокол поддерживает свой набор `DeviceCommand`. MKS - узкий (enable/disable, move, stop, read status, clear error). STM32 - широкий (configure, self_test, check, paper.*, table.*, guillotine.*, profile.run и т.п.).

Если `Scene` отправит команду, которую целевой протокол не поддерживает напрямую, поведение однозначное:

- `MksCan` либо раскладывает команду на свои поддерживаемые (если это возможно: например, `Stop` -> MKS-stop), либо возвращает `DeviceTaskStatus::Rejected`;
- `Scene` интерпретирует `Rejected` как ошибку конфигурации, а не как сбой устройства, и переводит state в `ERROR` с понятным сообщением.

Чтобы такие ошибки ловились на boot, а не на первом нажатии "старт", у каждого протокола есть статический capability-список:

```cpp
class MksCan {
public:
    static bool supports(DeviceCommand cmd);   // compile-time таблица
};
```

`DeviceRegistry::configureRequiredDevices()` при boot проверяет, что для каждой роли, используемой сценами, целевой device поддерживает нужные команды. Если нет - boot уходит в `ERROR` с сообщением вида "device 'paper' (mks) does not support PaperFeedUntilMark, нужен protocol stm.v1". Это смещает ошибку конфигурации с runtime на boot.

## StmCan

`StmCan` - основной будущий протокол для своих STM32-плат.

STM32-device должен поддерживать более крупные задания:

- выполнить проверку;
- принять конфигурацию;
- запустить сценарий;
- остановиться;
- вернуть статус;
- отправить событие.

Именно в STM32-device должна жить realtime-логика: ловля датчиков, энкодеры, точные остановки, синхронизация нескольких моторов.

## Heartbeat

Heartbeat - механизм определения "жив/не жив" и согласования версии протокола. Источник heartbeat зависит от типа device.

### Heartbeat для STM32-device (protocol `stm.v1`)

STM32-плата сама шлет heartbeat-кадр с фиксированной периодичностью.

- CAN ID: `0x700 + address` (диапазон `0x700..0x77F` зарезервирован под heartbeat, чтобы `CanRouter` отличал его по маске).
- Период: **200 мс**.
- Таймаут на стороне головы: **1000 мс** (5 пропусков подряд - device помечается offline). Берется из `can.heartbeat_timeout_ms`.
- Длина payload: 8 байт.

Раскладка payload (little-endian):

| байт | поле                | смысл                                                  |
|------|---------------------|--------------------------------------------------------|
| 0    | `protocolMajor`     | мажорная версия протокола (для `stm.v1` = 1)           |
| 1    | `protocolMinor`     | минорная версия                                        |
| 2    | `state`             | bitfield: bit0=ready, bit1=busy, bit2=error, bit3=safe |
| 3    | `errorCode`         | короткий код ошибки, 0 если нет                        |
| 4-5  | `activeTaskId`      | id текущего scene-task (0 если idle)                   |
| 6    | `firmwareMajor`     | мажорная версия прошивки                               |
| 7    | `firmwareMinor`     | минорная версия прошивки                               |

`type` device и его `name` в heartbeat не передаются - они известны из конфига по адресу. Это экономит байты и убирает двойной источник правды.

Версия протокола (`protocolMajor/Minor`) обязательна. `DeviceRegistry` сравнивает ее с ожидаемой из конфига:

- major не совпал -> device помечается `incompatible`, не `online`. `Scene` не отправляет ему task.
- minor отличается -> online, но в лог пишется warning.

### Heartbeat для MKS Servo57D_CAN (protocol `mks`)

У MKS своего heartbeat-кадра нет. Поэтому "жив/не жив" определяется через polling: `MksCan` сам шлет на каждый MKS-адрес запрос статуса (read status / read position) с тем же периодом **200 мс**, и обновляет `DeviceRegistry` по факту получения ответа в окне **1000 мс**.

Polling-период и таймаут берутся из тех же `can.heartbeat_timeout_ms` / `can.heartbeat_period_ms`, чтобы поведение online/offline было одинаковым для всех протоколов.

`protocolMajor/Minor` для MKS-устройства - константа в `MksCan`, не вычитывается из железа.

### Общее поведение

Если для device (любого протокола) истек таймаут - `DeviceRegistry` помечает его `offline`, активные task этого device переводит в `Failed/Timeout`, `Scene` получает соответствующий статус. Восстановление - по первому успешному heartbeat / polling-ответу.

## Критерий готовности

- Есть `ICanBus` и `Esp32TwaiBus`.
- Есть заготовки `MksCan` и `StmCan`.
- `DeviceRegistry` получает статусы через `CanRouter`.
- Можно отправить абстрактный `DeviceTask` конкретному адресу.
- Без подключенных device boot не падает, а показывает понятное состояние.

