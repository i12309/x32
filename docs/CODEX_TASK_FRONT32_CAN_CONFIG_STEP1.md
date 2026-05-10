# Задача для Codex: Front32 — первый шаг перехода на CAN-конфиги

## Контекст

Репозиторий: `https://github.com/i12309/Front32`

Проект уже начал переход от монолитного `config.device` к модели, где Front32 является головным CAN-устройством:

- головной `/config.json` хранит настройки Front32, CAN-шины, список логических нод и группы;
- каждая нода имеет отдельный файл `/node_<NAME>.json`;
- Front32 должен на этапе загрузки убедиться, что для выбранного типа станка перечислены все нужные ноды и группы;
- глубокую проверку содержимого `device` внутри нод сейчас не делать — это ответственность самих нод после получения своего конфига;
- старую монолитную проверку полного `device` через `MachineSpec::A` нужно заменить.

Сейчас `src/Machine/MachineSpec.*` и `src/Machine/Types/MachineSpec_A.cpp` описывают старый монолитный `device`: `I2C`, `motors`, `sensors`, `optical`, `switchs`, `buttons`, `encoders`. Это нужно переработать в спецификацию состава CAN-топологии станка.

## Цель этой задачи

Сделать первый безопасный шаг: централизованно загрузить и проверить головной CAN-конфиг и файлы нод, не реализуя пока BOOT discovery и передачу JSON по CAN.

После задачи загрузка `/config.json` должна падать с понятной диагностикой, если:

- в головном конфиге не перечислены обязательные ноды для выбранного типа станка;
- нет обязательной группы для синхронного движения PAPER+THROW;
- отсутствует файл `/node_<NAME>.json` для ноды из списка;
- файл ноды не JSON или не проходит базовую envelope-проверку;
- есть дубли имен нод, CAN ID, групповых CAN ID или реальных MAC;
- нода ссылается на group ID, которого нет в головном разделе `groups`.

## Ожидаемая схема головного конфига

Использовать такую форму json\config_CAN.json

## Ожидаемая базовая схема файла ноды

Файл `/node_<NAME>.json` должен проходить только envelope-проверку:

```json
{
  "config_version": 0,
  "machine": "NODE",
  "name": "PAPER",
  "group": "0x220",
  "mac": "00:00:00:00",
  "canID": "0x203",
  "settings": {},
  "CAN": {},
  "device": {}
}
```

Правила:

1. Файл должен существовать и быть валидным JSON.
2. `machine` строго `"NODE"`.
3. `name` должен быть непустым, уникальным, совпадать с именем в `nodes` и с ожидаемым именем файла.
4. `group` может быть `0`, `"0x000"` или отсутствовать — это значит «без группы».
5. Если `group` ненулевой, он должен быть валидным standard CAN ID `1..0x7FF` и должен существовать в головном `groups`.
6. `mac` должен быть строкой. Значение `"00:00:00:00"` разрешено как временная заглушка и не участвует в проверке уникальности. Любой другой MAC должен быть уникален.
7. `canID` должен быть валидным standard CAN ID `1..0x7FF` и уникальным среди нод. Желательно также не пересекать индивидуальные `canID` с group ID.
8. Полный JSON файла ноды нужно сохранить как compact payload-строку для будущей отправки по CAN.

## Архитектурное решение

### 1. Переработать `MachineSpec`

Файлы:

- `src/Machine/MachineSpec.h`
- `src/Machine/MachineSpec.cpp`
- `src/Machine/Types/MachineSpec_A.cpp`

Новый смысл `MachineSpec`: не список локального железа в `device`, а логическая CAN-топология для типа станка.

### 2. Спецификация для типа `A`

В `src/Machine/Types/MachineSpec_A.cpp` описать только логическую топологию.

Минимум для текущего кода:

```cpp
MachineSpec MachineSpec::makeA() {
    MachineSpec spec;
    spec.type_ = Catalog::MachineType::A;

    spec.requiredNodes_.push_back({"TABLE", true});
    spec.requiredNodes_.push_back({"GUILLOTINE", true});
    spec.requiredNodes_.push_back({"PAPER", true});
    spec.requiredNodes_.push_back({"THROW", true});

    spec.requiredGroups_. ... тут придумать по примеру ;

    return spec;
}
```

Проверка группы:

- ноды `PAPER` и `THROW` должны существовать;
- обе должны иметь ненулевой `groupID`;
- `groupID` должен быть одинаковым;
- этот group ID должен быть объявлен в головном `groups`.

и других групп по примеру 

### 3. Переработать загрузку в `Core::ConfigJson`

Файл: `src/Core.h`.

Сейчас в `ConfigJson` уже есть:

- `CanBusConfig can`;
- `parseCanID()`;
- заготовка `loadCanConfig()`;
- чтение `CAN.tx`, `CAN.rx`, `CAN.bitrate`, `CAN.timeouts_ms.ack/check`.

Нужно расширить это до полноценной загрузки.

Добавить структуры примерно такого вида:

```cpp
struct GroupConfig {
    String name;
    uint16_t id = 0;
};

struct NodeConfig {
    String name;
    String path;
    String mac;
    uint16_t canID = 0;
    uint16_t groupID = 0;
    String payload;   // compact serialized full node JSON
};

std::vector<GroupConfig> groups;
std::vector<NodeConfig> nodes;
```

Добавить helper-методы:

```cpp
bool loadCanConfig();
bool loadGroups(JsonObjectConst groupsObj);
bool loadNodes(JsonArrayConst nodeArray);
bool loadNodeFile(const String& expectedName, NodeConfig& out);
bool hasGroupID(uint16_t id) const;
bool findNode(const String& name, NodeConfig& out) const;
bool nodeAddress(const char* name, uint16_t& out) const;
bool nodeGroup(const char* name, uint16_t& out) const;
```

Можно оставить часть методов приватными. Публичными нужны только те, которые будет использовать `CAN.cpp`.

`loadCanConfig()` должен:

1. Сбрасывать `can`, `groups`, `nodes` перед загрузкой.
2. Загрузить `CAN.tx`, `CAN.rx`, `CAN.bitrate`, `CAN.timeouts_ms.ack`, `CAN.timeouts_ms.check`.
3. Проверить `bitrate` на допустимые значения `125`, `250`, `500`, `1000`.
4. Проверить, что `nodes` — непустой массив строк.
5. Проверить дубли имен в `nodes`.
6. Прочитать `groups` как объект `name -> canID`.
7. Для каждой ноды из `nodes` открыть `/node_<NAME>.json`, проверить envelope и сохранить payload.
8. Проверить дубли `canID`, дубли group ID, пересечение node `canID` с group ID, дубли реальных MAC.
9. В конце вызвать `MachineSpec::get(Catalog::getMachine(machine)).validateControllerConfig(doc.as<JsonObjectConst>(), nodeInfos)` и, если есть errors, вернуть `false`.

Важно: при ошибке писать понятное сообщение через `Log::E`/`Log::D`: что именно сломано, имя ноды, путь файла, поле, конфликтующее значение.

### 4. Обновить `ConfigDefaults`

Файлы:

- `src/config.h`
- `src/config.cpp`

`ConfigDefaults::build()` сейчас создает монолитный `device` через `MachineSpec::fillDeviceDefaults()`.

Нужно заменить на дефолт головного устройства:

- оставить `config_version`, `machine`, `name`, `settings`, `tuning`;
- добавить секцию `CAN` с дефолтами `tx=17`, `rx=18`, `bitrate=500`, `timeouts_ms.ack=150`, `timeouts_ms.check=300`;
- добавить `nodes` для типа `A`: `TABLE`, `GUILLOTINE`, `PAPER`, `THROW`;
- добавить `groups`: `FEED_THROW = "0x220"`;
- не создавать `device` в головном config.

Если нужен метод заполнения дефолтов через `MachineSpec`, использовать новый `fillControllerDefaults(JsonObject root)`.

### 5. Обновить `CAN.cpp`, чтобы не читать файлы нод заново

Файл: `src/Service/CAN.cpp`.

Сейчас `CAN.cpp` локально строит путь `/node_<NAME>.json`, читает JSON ноды из LittleFS и достает `canID/group` каждый раз, когда нужен адрес.

После централизованной загрузки нужно заменить это на чтение из `Core::config.nodes`:

- `CAN::nodeAddress()` должен брать адрес из `Core::config.nodeAddress(...)` или аналогичного метода.
- `CAN::groupFeedThrowAddress()` должен брать group ID из уже загруженных данных.
- Локальные helper-функции `nodePath()`, `loadNodeDoc()`, `nodeCanID()`, `nodeGroupID()` удалить или больше не использовать.

Это не меняет публичное поведение `CAN`, но убирает повторное чтение файлов и гарантирует, что все ошибки конфигурации обнаруживаются в BOOT, а не во время движения.

### 6. Обновить примеры JSON

Проверить папку `json/`.

Должны быть валидные примеры:

- `json/config_A.json` — теперь головной CAN-конфиг, не монолитный `device`;
- `json/node_TABLE.json`;
- `json/node_GUILLOTINE.json`;
- `json/node_PAPER.json`;
- `json/node_THROW.json`.

Убедиться, что:

- `config_A.json.nodes` содержит все четыре ноды;
- `config_A.json.groups.FEED_THROW` равен `0x220`;
- `node_PAPER.json.group` и `node_THROW.json.group` равны `0x220`;
- все `canID` уникальны;
- `mac: "00:00:00:00"` не считается конфликтом.

### 7. Диагностика и ошибки

Примеры сообщений:

```text
[Config] nodes must be a non-empty array
[Config] duplicate node name in config.nodes: PAPER
[Config] node file is missing: /node_TABLE.json
[Config] node /node_TABLE.json has machine='A', expected 'NODE'
[Config] node name mismatch: list='TABLE', file='PAPER'
[Config] duplicate CAN ID 0x203: PAPER and TABLE
[Config] node PAPER uses group 0x220 but config.groups does not declare it
[MachineSpec] required node is missing for machine A: THROW
[MachineSpec] required group FEED_THROW needs PAPER and THROW in the same non-zero group
```

Ошибки должны блокировать загрузку. Warnings можно использовать только для некритичных будущих случаев.

## Acceptance criteria

1. Проект собирается для `env:esp32dev`.
2. `Core::config.load(false)` успешно загружает корректный головной CAN-конфиг и все `/node_<NAME>.json`.
3. После успешной загрузки в памяти доступны:
   - `Core::config.can`;
   - список загруженных нод с `name`, `canID`, `groupID`, `mac`, `payload`;
   - список групп.
4. `MachineSpec::A` больше не проверяет монолитный `device`; он проверяет обязательные ноды и группу `FEED_THROW`.
5. `CAN.cpp` получает адреса нод из уже загруженного `Core::config`, а не перечитывает JSON-файлы с LittleFS на каждый вызов.
6. `CAN.enabled` нигде не добавлен.
7. `mac: "00:00:00:00"` разрешен и не считается дублем.
8. Реальные MAC считаются уникальными.
9. Ненулевой `group` в файле ноды должен присутствовать в головном `groups`.
10. Индивидуальные `canID` нод уникальны и не равны group ID.
11. Для типа `A` отсутствие `TABLE`, `GUILLOTINE`, `PAPER` или `THROW` блокирует загрузку.
12. Для типа `A` `PAPER` и `THROW` должны иметь одинаковую ненулевую группу.
13. При ошибке в конфиге BOOT должен остановиться так же, как сейчас останавливается при ошибке `Core::config.load`, но сообщение должно указывать конкретную причину.

## Проверки плохих конфигов

Нужно вручную или тестово проверить такие случаи:

- нет секции `nodes`;
- `nodes` пустой;
- `nodes` содержит дубль;
- в `nodes` есть `TABLE`, но файла `/node_TABLE.json` нет;
- файл ноды пустой;
- файл ноды невалидный JSON;
- `machine` в файле ноды не `NODE`;
- `name` в файле ноды не совпадает с именем из `nodes`;
- `canID` отсутствует, равен `0`, больше `0x7FF` или не парсится;
- две ноды имеют одинаковый `canID`;
- две ноды имеют одинаковый реальный MAC;
- две ноды имеют `00:00:00:00` — это должно быть разрешено;
- `group` ноды ненулевой, но не объявлен в `config.groups`;
- `PAPER` и `THROW` в разных группах;
- `PAPER` или `THROW` без группы.

## Команда сборки

По правилам проекта можно попробовать:

```powershell
C:\WINDOWS\System32\WindowsPowerShell\v1.0\powershell.exe -Command '& "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe" run'
```

Если среда недоступна, в ответе указать, что сборку локально запустить не удалось, и перечислить внесенные изменения.

## Важное по стилю

- Комментарии и диагностические сообщения писать по-русски, если это не ломает уже существующий стиль конкретного файла.
- Следить за кодировкой: не допускать mojibake.
- Для Markdown использовать UTF-8, предпочтительно BOM.
- Для исходников сохранять текущую кодировку файла.
