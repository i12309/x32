# Этап 04: доменные страницы

## Цель

Перенести рабочие разделы старого `src/UI` на LVGL runtime: задания, профили, сервис, Wi-Fi, статистику и поддержку.

Правила остаются такими же, как в этапах 02-03:

- без `PageManager`;
- без временных compatibility facade;
- без wrapper-классов и генераторов заранее;
- события через `Ui::onPop(...)` / `Ui::onPush(...)`;
- данные через `objects.*` и минимальные helpers в `LvglHelpers.h`;
- если нужный объект в EEZ имеет техническое имя `objNN`, оставить `TODO(ui-lvgl)` и пропустить эту часть до переименования объекта в EEZ.

## Структура файлов

Все новые page-классы кладутся в папки, соответствующие старой структуре `src/UI`:

- `src/UI/Task/*` -> `src/Screen/Page/Task/*`;
- `src/UI/Profile/*` -> `src/Screen/Page/Profile/*`;
- `src/UI/Service/*` -> `src/Screen/Page/Service/*`;
- `src/UI/Wifi/*` -> `src/Screen/Page/Wifi/*`;
- `src/UI/Statistics/*` -> `src/Screen/Page/Statistics/*`;
- `src/UI/Help/*` -> `src/Screen/Page/Help/*`.

Пример:

```cpp
namespace Screen {

class TaskRun : public Page {
public:
    static TaskRun& instance();

private:
    TaskRun() : Page(SCREEN_ID_TASK_RUN) {}
};

}  // namespace Screen
```

## Универсальные страницы

Не каждую старую Nextion-страницу нужно переносить в отдельную LVGL-страницу один к одному.

### `SCREEN_ID_LIST`

`SCREEN_ID_LIST` используется как универсальная страница списка.

Создать обезличенный каркасный класс:

- `src/Screen/Page/Task/List.h|cpp`;
- класс `Screen::List`;
- `List() : Page(SCREEN_ID_LIST) {}`.

`Screen::List` не содержит бизнес-логики заданий, профилей или статистики. Это только инструмент для отрисовки строк, кнопок, чекбоксов, edit/delete/add/next/back и общего состояния generated страницы `List`.

Примерный каркас:

```cpp
struct ListRow {
    String text;
    bool checked = false;
    bool checkVisible = true;
    bool editVisible = true;
    void* userData = nullptr;
};

struct ListCallbacks {
    lv_event_cb_t onBack = nullptr;
    lv_event_cb_t onAdd = nullptr;
    lv_event_cb_t onDelete = nullptr;
    lv_event_cb_t onNext = nullptr;
    lv_event_cb_t onRow = nullptr;
    lv_event_cb_t onEdit = nullptr;
    lv_event_cb_t onCheck = nullptr;
};

class List : public Page {
public:
    void showList(const String& title,
                  const ListRow* rows,
                  size_t rowCount,
                  const ListCallbacks& callbacks);
};
```

Отдельные доменные классы остаются обязательными:

- `Screen::TaskList` содержит бизнес-логику старого `pTaskList`;
- `Screen::ProfileList` содержит бизнес-логику старого `pProfileList`;
- оба используют `Screen::List` как отрисовочный каркас.

Пример:

```cpp
void TaskList::show() {
    buildRowsFromTasks();
    Screen::List::instance().showList("Задания", rows, rowCount, callbacks);
}
```

Новые поля и кнопки generated страницы `List`, которым пока нет смысла в старом UI, можно игнорировать. Если поле понадобится, но объект называется `objNN`, оставить TODO и пропустить.

### `SCREEN_ID_PAGE`

`SCREEN_ID_PAGE` используется как универсальная страница меню раздела.

Она нужна как каркас для однотипных экранов, которые в старом UI были отдельными страницами, но по структуре являются меню:

- `src/UI/Statistics/pStatistics.*`;
- `src/UI/Help/pHelp.*`;
- другие похожие меню разделов, если появятся.

Создать обезличенный каркасный класс:

- `src/Screen/Page/Help/Page.h|cpp`;
- класс `Screen::PageMenu` или `Screen::SectionPage`;
- `PageMenu() : Page(SCREEN_ID_PAGE) {}`.

Добавить универсальный метод:

```cpp
struct MenuItem {
    String title;
    lv_event_cb_t onPop;
    bool visible = true;
};

void showMenu(const String& title,
              const MenuItem* items,
              size_t itemCount,
              lv_event_cb_t onBack = nullptr);
```

`showMenu(...)` должен:

- открыть `SCREEN_ID_PAGE`;
- поставить заголовок;
- показать нужные пункты;
- скрыть неиспользуемые пункты;
- повесить callbacks;
- не падать, если часть объектов пока имеет технические имена. В этом случае оставить TODO и пропустить настройку этого поля.

`PageMenu` не знает, что такое статистика, помощь, лицензия или обновление. Конкретные доменные классы остаются отдельными:

- `Screen::Statistics` содержит бизнес-логику старого `pStatistics`;
- `Screen::Help` содержит бизнес-логику старого `pHelp`;
- они используют `PageMenu::showMenu(...)` как каркас.

Пример:

```cpp
void Statistics::show() {
    const PageMenu::MenuItem items[] = {
        {"Устройство", Statistics::popDevice},
        {"Задания", Statistics::popTasks},
        {"Профили", Statistics::popProfiles},
    };

    PageMenu::instance().showMenu("Статистика", items, 3, Statistics::popBack);
}
```

Суть меню можно дошлифовать позже. На этом этапе важно убрать дублирование отрисовки, но не смешивать бизнес-логику разных разделов в одном классе.

## Группы переноса

### Task

Старые файлы:

- `src/UI/Task/pTaskRun.*`;
- `src/UI/Task/pTaskProcess.*`;
- `src/UI/Task/pTask.*`;
- `src/UI/Task/pTaskList.*`.

Новые файлы:

- `src/Screen/Page/Task/TaskRun.h|cpp`;
- `src/Screen/Page/Task/TaskProcess.h|cpp`;
- `src/Screen/Page/Task/Task.h|cpp`;
- `src/Screen/Page/Task/TaskList.h|cpp`;
- `src/Screen/Page/Task/List.h|cpp`.

Generated screens:

- `SCREEN_ID_TASK_RUN`;
- `SCREEN_ID_TASK_PROCESS`;
- `SCREEN_ID_TASK`;
- `SCREEN_ID_LIST`.

Работы:

- перенести callbacks кнопок;
- перенести выбор задания и профиля;
- перенести изменение cycles;
- сохранить обработку физической кнопки `START`, если она есть в старом сценарии;
- бизнес-логику списка заданий держать в `TaskList`;
- `TaskList` использует `List::showList(...)` только для отрисовки;
- переходы к еще не перенесенным страницам временно закомментировать с `TODO(ui-lvgl)`.

### Profile

Старые файлы:

- `src/UI/Profile/pProfile.*`;
- `src/UI/Profile/pProfileList.*`.

Новые файлы:

- `src/Screen/Page/Profile/Profile.h|cpp`.
- `src/Screen/Page/Profile/ProfileList.h|cpp`.

Generated screens:

- `SCREEN_ID_PROFILE`;
- `SCREEN_ID_LIST` через каркас `Screen::List`.

`ProfileList` как доменный класс создать обязательно. Он содержит бизнес-логику старого `pProfileList`, а `Screen::List` использует только как отрисовочный инструмент.

### Service

Старые файлы:

- `src/UI/Service/pService.*`;
- `src/UI/Service/pService2.*`;
- `src/UI/Service/pTable.*`;
- `src/UI/Service/pPaper.*`;
- `src/UI/Service/pGuillotine.*`;
- `src/UI/Service/pThrow.*`;
- `src/UI/Service/pBigel.*`;
- `src/UI/Service/pCalibration.*`;
- `src/UI/Service/pSlice.*`;
- `src/UI/Service/pKnife.*`.

Новые файлы:

- `src/Screen/Page/Service/Service.h|cpp`;
- `src/Screen/Page/Service/Service2.h|cpp`;
- `src/Screen/Page/Service/Table.h|cpp`;
- `src/Screen/Page/Service/Paper.h|cpp`;
- `src/Screen/Page/Service/Guillotine.h|cpp`;
- `src/Screen/Page/Service/Throws.h|cpp`;
- `src/Screen/Page/Service/Bigel.h|cpp`;
- `src/Screen/Page/Service/Calibration.h|cpp`;
- `src/Screen/Page/Service/Slice.h|cpp`.

Generated screens:

- `SCREEN_ID_SERVICE`;
- `SCREEN_ID_SERVICE2`;
- `SCREEN_ID_TABLE`;
- `SCREEN_ID_PAPER`;
- `SCREEN_ID_GUILLOTINE`;
- `SCREEN_ID_THROWS`;
- `SCREEN_ID_BIGEL`;
- `SCREEN_ID_CALIBRATION`;
- `SCREEN_ID_SLICE`.

`pKnife` переносить только если для него есть реальный LVGL screen или он нужен как часть другой страницы. Если отдельного экрана нет, оставить TODO и не создавать пустой класс.

### Wifi

Старые файлы:

- `src/UI/Wifi/pWiFi.*`;
- `src/UI/Wifi/pSettings.*`.

Новые файлы:

- `src/Screen/Page/Wifi/Wifi.h|cpp`;
- `src/Screen/Page/Wifi/Settings.h|cpp`, только если в EEZ есть отдельный экран или явный сценарий.

Generated screen:

- `SCREEN_ID_WIFI`.

Если `pSettings` был только логическим подэкраном без отдельной generated страницы, не создавать отдельный page-класс. Перенести его поведение в `Wifi`.

### Statistics

Старые файлы:

- `src/UI/Statistics/pStatistics.*`;
- `src/UI/Statistics/pShowStat.*`.

Новые файлы:

- `src/Screen/Page/Statistics/Statistics.h|cpp`;
- `src/Screen/Page/Statistics/ShowStat.h|cpp`;
- меню `pStatistics` перенести в `Screen::Statistics`;
- `Screen::Statistics` использует универсальный `Screen::PageMenu::showMenu(...)`.

Generated screens:

- `SCREEN_ID_STATS`;
- `SCREEN_ID_PAGE`.

Если `SCREEN_ID_STATS` уже удобнее для `ShowStat`, использовать его для таблицы статистики. Если нет, временно оставить `ShowStat` на `SCREEN_ID_STATS` и меню на `SCREEN_ID_PAGE`.

### Help

Старые файлы:

- `src/UI/Help/pHelp.*`;
- `src/UI/Help/pUpdate.*`;
- `src/UI/Help/pLicence.*`;
- `src/UI/Help/pDevice.*`;
- `src/UI/Help/pParams.*`;
- `src/UI/Help/PinTest.*`.

Новые файлы:

- `src/Screen/Page/Help/Page.h|cpp` для универсального меню `SCREEN_ID_PAGE`;
- `src/Screen/Page/Help/Help.h|cpp`;
- `src/Screen/Page/Help/Update.h|cpp`;
- `src/Screen/Page/Help/Licence.h|cpp`;
- `src/Screen/Page/Help/Device.h|cpp`;
- `src/Screen/Page/Help/Params.h|cpp`;
- `src/Screen/Page/Help/PinTest.h|cpp`, только если есть подходящий LVGL screen.

Generated screens:

- `SCREEN_ID_PAGE`;
- `SCREEN_ID_UPDATE`;
- при необходимости другие существующие generated screens.

`Help` как доменный класс создать обязательно. Он содержит бизнес-логику старого `pHelp`, а `PageMenu` использует только как отрисовочный инструмент.

## Helpers

В ходе переноса расширять только `src/Screen/Panel/LvglHelpers.h`.

Ожидаемо понадобятся:

- `Ui::setText(...) -> String API уже есть`;
- `Ui::getText(...) -> String уже есть`;
- `Ui::setHidden(...)`;
- `Ui::setChecked(...)`, `Ui::isChecked(...)`;
- `Ui::dropdownSetOptions(...)`;
- `Ui::dropdownSelected(...)`;
- `Ui::setEnabled(...)`;
- `Ui::setValue(...)`, `Ui::getValue(...)` для spinbox/slider/textarea, только когда реально понадобится.

Не добавлять большие wrapper-объекты, модели и генераторы, пока без них можно перенести страницу понятно и коротко.

## Правила переноса

1. Сначала найти старую страницу `src/UI/<Folder>/p*.h|cpp`.
2. Найти generated объекты в `lib/eez_ui/src/ui/screens.h`.
3. Создать новый класс в соответствующей папке `src/Screen/Page/<Folder>/`.
4. В `onPrepare()` повесить события через `Ui::onPop(...)`.
5. В `onShow()` заполнить экран из текущего состояния.
6. В `onTick()` оставить только периодическое обновление активной страницы.
7. Если старый код читал состояние из цвета/видимости Nextion-объекта, заменить на явное поле класса.
8. Если нужного named object нет, оставить `TODO(ui-lvgl)` и не трогать generated `screens.c`.
9. Старые вызовы на еще не перенесенные страницы временно закомментировать с TODO. Facade не делать.

## Проверки

- `pio run -e JC8048W550C` проходит после каждой группы.
- `TaskRun` открывает выбор задания через `TaskList`, который использует `List::showList(...)`.
- `TaskRun` открывает выбор профиля через `ProfileList`, который использует `List::showList(...)`.
- `Profile` может открывать `ProfileList`.
- `Statistics` и `Help` используют один отрисовочный каркас `PageMenu::showMenu(...)`, но бизнес-логика остается в `Statistics` и `Help`.
- `Stats`, `Wifi`, `Update`, `Help` не используют Nextion UART.
- В новых файлах нет `Nex*`, `nexLoop`, `sendCommand`, `PageManager`, `getHMIVersion`.

## Риски

- Универсальные `List` и `PageMenu` должны оставаться обезличенными. Если в них начинает попадать бизнес-логика заданий, профилей, статистики или помощи, ее нужно вернуть в доменный класс.
- Новые поля и кнопки generated страниц пока можно игнорировать.
- В EEZ часть важных элементов может называться `objNN`. В таком месте нужно оставить TODO и вернуться после переименования объекта в EEZ.
- Старые физические кнопки, например `START`, нужно сохранить отдельно от touch callbacks.
