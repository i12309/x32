# Этап 4. Новый config и DeviceRegistry

## Задача этапа

Сделать конфиг, который описывает соседей по CAN и параметры, которые головное устройство должно передать этим соседям.

Головное устройство должно понимать:

- какие device есть на шине;
- какие у них адреса;
- какой протокол использовать;
- какие device обязательны для работы;
- какую секцию конфигурации отправить каждому device.

Головное устройство не должно знать внутреннюю realtime-схему соседнего device. Но оно может хранить и передать соседу его конфигурацию.

## Новый принцип config

Имена соседей должны быть короткими и соответствовать роли:

- `paper`;
- `motion`;
- `panel`;
- `guillotine`, если это отдельный device;
- `table`, если это отдельный device.

Не использовать искусственные имена вида `paper_unit`, если достаточно `paper`.

В описании device не нужно одновременно держать `type` и `protocol`. Пока достаточно `protocol`, потому что именно он определяет, какой CAN-класс будет общаться с устройством.

Не использовать вложенную секцию `settings` для параметров, отправляемых соседу. Лучше использовать вложенную секцию `device`, потому что она по смыслу похожа на текущую секцию `device` из `config_A.json`: это конфигурация оконечного устройства.

## Пример описания одного device

```json
"paper": {
  "address": 33,
  "protocol": "mks",
  "required": true,
  "device": {
    "role": "paper"
  }
}
```

Для STM32-device секция `device` может быть похожа на нынешнюю `config_A.json`, только это уже конфиг не головного ESP32, а оконечной платы.

Например:

```json
"motion": {
  "address": 16,
  "protocol": "stm.v1",
  "required": true,
  "device": {
    "motors": {
      "TABLE": {
        "driver": "RMT",
        "pin": {"step": 23, "dir": 0, "ena": 1},
        "MicroStep": 3200,
        "WorkMove": 9500
      },
      "GUILLOTINE": {
        "driver": "RMT",
        "pin": {"step": 18, "dir": 4, "ena": 5},
        "MicroStep": 3200,
        "WorkMove": 3200
      }
    },
    "sensors": {
      "TABLE_UP": {"pin": 0},
      "TABLE_DOWN": {"pin": 1},
      "GUILLOTINE": {"pin": 2}
    }
  }
}
```

Головное устройство хранит эту секцию и передает ее в `motion`, но не использует эти пины для собственной realtime-логики.

## Черновик config

```json
{
  "config_version": 1,
  "machine": "A",
  "name": "HEAD_JC8048",
  "group": "DEV",
  "settings": {
    "AUTO_UPDATE": 0,
    "CONNECT_WIFI": 1,
    "HTTP_SERVER": 1,
    "CHECK_SYSTEM": 1,
    "ALLOW_MISSING_HARDWARE": 0,
    "log": 1,
    "metrics": 1
  },
  "can": {
    "driver": "twai",
    "tx_pin": 17,
    "rx_pin": 18,
    "bitrate": 500000,
    "mode": "normal",
    "heartbeat_period_ms": 200,
    "heartbeat_timeout_ms": 1000,
    "task_timeout_ms": 5000
  },
  "devices": {
    "paper": {
      "address": 33,
      "protocol": "mks",
      "required": true,
      "device": {
        "role": "paper",
        "note": "Формат параметров MKS Servo57D_CAN надо уточнить отдельно."
      }
    },
    "motion": {
      "address": 16,
      "protocol": "stm.v1",
      "required": true,
      "device": {
        "I2C": {
          "MCP0": {"type": "output", "address": "0x20"},
          "MCP1": {"type": "output", "address": "0x21"},
          "MCP2": {"type": "input", "address": "0x22", "intA": 39, "intB": 34}
        },
        "motors": {
          "TABLE": {
            "I2C": "MCP0",
            "driver": "RMT",
            "pin": {"step": 23, "dir": 0, "ena": 1},
            "MicroStep": 3200,
            "WorkMove": 9500,
            "Speed": [
              {"Mode": "Normal", "Speed": 16000, "Acceleration": 32000},
              {"Mode": "Slow", "Speed": 9600, "Acceleration": 9600}
            ]
          },
          "GUILLOTINE": {
            "I2C": "MCP0",
            "driver": "RMT",
            "pin": {"step": 18, "dir": 4, "ena": 5},
            "MicroStep": 3200,
            "WorkMove": 3200
          }
        },
        "sensors": {
          "TABLE_UP": {"I2C": "MCP2", "pin": 0},
          "TABLE_DOWN": {"I2C": "MCP2", "pin": 1},
          "GUILLOTINE": {"I2C": "MCP2", "pin": 2}
        }
      }
    },
    "panel": {
      "address": 48,
      "protocol": "stm.v1",
      "required": false,
      "device": {
        "buttons": {
          "START": {"pin": 16}
        }
      }
    }
  }
}
```

Пины CAN для головного устройства: `tx_pin = 17`, `rx_pin = 18`.

## Что важно про секцию device

`devices.<name>.device` - это не конфигурация головного ESP32.

Это payload для соседнего device:

- BOOT читает эту секцию;
- `DeviceRegistry` хранит ее в `DeviceNode`;
- при `configure` эта секция отправляется соседу;
- сосед сам решает, как создать свои моторы, датчики, энкодеры, MCP и т.п.

Для STM32-плат эта секция может быть очень похожа на текущую `config_A.json/device`, потому что именно STM32 будет оконечным realtime-контроллером.

Для MKS Servo57D_CAN формат секции пока неизвестен. В конфиге нужно оставить минимальный блок и отдельно уточнить, какие параметры MKS реально принимает или требует.

## Role и DeviceName - две разные вещи

В коде явно разделяются два понятия:

- `DeviceName` - имя ноды в конфиге (`paper`, `motion`, `panel`). Это идентификатор записи в `DeviceRegistry`. Однозначно соответствует одному CAN-адресу.
- `Role` - логическая функция, которую сцена ожидает увидеть на каком-то device (`Paper`, `Table`, `Guillotine`, `Check`). Сцена работает в терминах ролей, а не имен.

Сейчас имена в конфиге совпадают с ролями (`paper` - и имя, и роль), но это совпадение случайное. Как только `guillotine` переедет с `motion` на отдельный device, или появится второй `paper` для другого формата - имя и роль разойдутся. Поэтому API делается сразу с разделением, чтобы потом не переписывать.

```cpp
enum class Role : uint8_t {
    Paper,
    Table,
    Guillotine,
    Panel,
    Motion
};

using DeviceName = const char*;     // ссылка в config, не владеет строкой
```

Маппинг роли в device описывается в конфиге секцией `roles` (опциональная, по умолчанию роль ищется по совпадению имени):

```json
"roles": {
  "paper":      "paper",
  "table":      "motion",
  "guillotine": "motion"
}
```

## DeviceRegistry

Новый `DeviceRegistry`:

- читает `devices`;
- создает записи `DeviceNode`;
- хранит `address`, `protocol`, `required`;
- хранит вложенную секцию `device` как конфигурацию для отправки соседу;
- отмечает online/offline по heartbeat;
- хранит текущие task id и статусы;
- разрешает `Role` в `DeviceNode` через секцию `roles` из конфига;
- дает `Scene` методы отправки задания по роли или по имени.

Минимальные методы:

```cpp
class DeviceRegistry {
public:
    bool loadFromConfig(JsonObjectConst root);   // читает devices и roles
    bool configureRequiredDevices();

    // Адресация по роли - основной путь для Scene.
    DeviceTaskId sendTask(Role role, DeviceCommand cmd, const DeviceParams& p, uint32_t timeoutMs);

    // Адресация по имени - для BOOT, диагностики, broadcast по списку.
    DeviceTaskId sendTask(DeviceName name, DeviceCommand cmd, const DeviceParams& p, uint32_t timeoutMs);

    DeviceTaskStatus taskStatus(DeviceTaskId id) const;
    DeviceStatus     status(DeviceName name) const;
    DeviceStatus     status(Role role) const;
    bool             allRequiredReady() const;
};
```

Никаких `String` в API. `DeviceName` - указатель на строку из распарсенного config (живет столько же, сколько `Core::config`). `Role` - enum.

Это не `MotorProxy` и не `SensorProxy`. Это реестр абстрактных соседей.

## Как Scene выбирает device

Сцены работают только в терминах `Role`:

- `PaperScene::feed()` -> `registry.sendTask(Role::Paper, DeviceCommand::PaperFeed, ...)`;
- `TableScene::up()` -> `registry.sendTask(Role::Table, DeviceCommand::TableUp, ...)`;
- `GuillotineScene::cut()` -> `registry.sendTask(Role::Guillotine, DeviceCommand::GuillotineCut, ...)`;
- `CheckScene::all()` -> отправка `Check` всем required device по списку имен.

Какой именно device сейчас закрывает роль `Guillotine` - решает `roles` в конфиге. Сцена этого не знает.

## MachineSpec

Пока оставить пустым или soft-check:

- не падать из-за отсутствия старой секции `device.motors` в корне;
- не требовать `PAPER`, `TABLE`, `GUILLOTINE` как отдельные моторы головного ESP32;
- максимум проверять, что в config есть хотя бы один required device.

## Критерий готовности

- `Core::config.load()` читает `config_version: 1`.
- В `devices` используются короткие имена: `paper`, `motion`, `panel`.
- В описании device используется `protocol`, без отдельного `type`.
- Параметры для соседей лежат в `devices.<name>.device`, а не в `settings`.
- `DeviceRegistry` создает список device.
- BOOT видит required device и их статусы.
- BOOT может отправить `devices.<name>.device` соседу командой `configure`.
- `Scene` может найти нужный device по короткому имени из `devices`.
- Старый детальный registry локальных моторов не нужен в головном режиме.
