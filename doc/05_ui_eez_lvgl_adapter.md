# Этап 5. Screen: EEZ/LVGL без Nextion backend

## Задача этапа

Перевести экранный код с Nextion на EEZ/LVGL для JC8048W550C и вынести его в новую папку `src/Screen`.

Переходный `NextionUiBackend` делать не нужно. Но код должен собираться даже пока перенесены не все страницы.

## Новая структура

Группировку страниц нужно сохранить близкой к текущей `src/UI`: `Main`, `Task`, `Service`, `Wifi`, `Statistics`, `Profile`, `Help` и т.п.

Проект должен быть самодостаточным: все нужные части из соседних проектов надо скопировать внутрь `x32`, а не подключать по абсолютным путям.

Рекомендуемая раскладка:

```text
src/Screen/
  Panel/
      ...
  Page/
    ...

lib/ScreenUI/
  eez_project/
    SMIT_v1.eez-project
    src/ui/
      ui.h/.c
      screens.h/.c
      styles.h/.c
      images.h/.c
      fonts.h
      actions.h
      vars.h
      ...
  generated/
    shared/
      page_ids.generated.h
      element_ids.generated.h
      page_descriptors.generated.h
      element_descriptors.generated.h
    frontend_meta/
      ui_object_map.generated.h/.cpp
      eez_page_meta.generated.cpp
  adapter/
    lvgl_eez/
      EezPanelMap.h/.cpp или упрощенный локальный adapter
  tools/
    ui_meta_gen/
      generate_ui_meta.py
      synced_attrs.yaml

lib/esp32-smartdisplay/
  только нужные драйверы и include для JC8048W550C

boards/
  JC8048W550C.json

include/
  lv_conf.h
```

`ScreenUI` внутри `lib/ScreenUI` должен быть локальной копией нужных частей, а не submodule/ссылка на соседний каталог. Если при переносе станет ясно, что часть `adapter` или generator-кода слишком сложная для `x32`, ее надо упростить, оставив понятный локальный слой.

Меняется корень и имена классов/файлов:

- было: `src/UI/Main/pLoad.h`, `pLoad`;
- станет: `src/Screen/Page/Main/Load.h`, `Load`;
- было: `src/UI/Main/pMain.h`, `pMain`;
- станет: `src/Screen/Page/Main/Main.h`, `Main`;
- было: `src/UI/Main/pINFO.h`, `pINFO`;
- станет: `src/Screen/Page/Main/Info.h`, `Info`.

```text
src/Screen/
  Panel/
      ...
  Page/
    Page.h/.cpp
    Main/

    Task/

    Service/

      ...
```

Если при реализации потребуется C++-тип в PascalCase, можно сделать внутренний тип `Load`, но публичное имя страницы и файл должны быть без префикса `p`: `Load`, `Main`, `Info`, `Error`.

Папки нужны только для физической организации файлов. Публичный доступ к страницам должен быть коротким:

```cpp
Screen::Load::getInstance().show();
Screen::Main::getInstance().show();
Screen::Info::showInfo(...);
```

Не использовать в рабочем коде длинные имена вида `Screen::Page::Main::load`.

## Разделение ответственности

`Screen/Page` - бизнес-логика страниц:

- какие данные показать;
- какие кнопки доступны;
- как реагировать на пользовательское действие;
- какие методы нужны остальному коду: `show()`, `text()`, `showInfo()`, `setProgressColor()`.

`Screen/Panel` - управление экраном:

- init дисплея JC8048W550C;
- init touch;
- init LVGL;
- init EEZ generated UI;
- общий `process()` / `tick`;
- показ страницы по имени/модели;
- установка текста, цвета, visible, value через объектную модель;
- доставка событий LVGL/EEZ в нужную страницу.

`Screen/Panel/Model` - объектная модель экрана:

- именованные страницы;
- именованные элементы внутри страниц;
- скрытая привязка к EEZ/LVGL/generated ids;
- typed-методы для работы с элементами.

`Page` не должен включать `Nextion.h`, не должен знать детали LVGL-драйвера и не должен использовать числовые id элементов.

`Panel` не должен содержать бизнес-логику станка. Он обслуживает экран, держит объектную модель и доставляет события страницам.

## Правило по id элементов

В page-классах нельзя обращаться к элементам по id.

Плохо:

```cpp
Panel::setText(txt_LOAD_VERSION, version);
Panel::setColor(pnl_LOAD_MODEL, color);
```

Правильно:

```cpp
auto& m = Screen::Panel::model().main.load;
m.version.setText(version);
m.modelStatus.setColor(color);
```

Все id из EEZ generated (`txt_LOAD_VERSION`, `pnl_LOAD_MODEL` и т.п.) должны быть спрятаны внутри `Screen/Panel/Model` или adapter/object-map слоя.

Page-класс должен работать с готовой объектной моделью страницы. Несмотря на то, что файл лежит в `Screen/Page/Main/load.h`, публичный класс находится прямо в namespace `Screen`:

```cpp
namespace Screen {

class load : public Page {
public:
    void show() override {
        panel().show(model().main.load);
    }

    void text(const String& value) {
        model().main.load.statusText.setText(value);
    }

    void setProgressColor(int index, uint32_t color) {
        model().main.load.progress[index].setColor(color);
    }
};

}
```

## Что сохранить

Старые страницы остаются как удобный фасад, но:

- переезжают в `Screen/Page`;
- сохраняют текущую группировку папок;
- теряют префикс `p`;
- перестают знать id элементов;
- работают через объектную модель страницы.

Остальной код после миграции должен выглядеть примерно так:

```cpp
Screen::load::getInstance().show();
Screen::load::getInstance().text("...");
Screen::info::showInfo(...);
```

На переходный период можно оставить compatibility aliases:

```cpp
using pLoad = Screen::load;
using pINFO = Screen::info;
```

Но новый код должен использовать имена без `p`.

## Что убрать

- `Page.h` не должен включать `Nextion.h`;
- страницы не должны хранить `NexPage`, `NexText`, `NexButton`;
- страницы не должны обращаться к `element_id`;
- BOOT не должен обновлять `.tft`;
- `NexUpdate` и `NexUploadHTTP` не нужны в головной сборке.

## Минимальный Panel API

```cpp
namespace Screen {

class Panel {
public:
    bool init();
    void process();

    ScreenModel& model();

    bool show(PageModel& page);
    bool apply(ElementModel& element);
};

}
```

Низкоуровневые методы по id могут существовать только внутри `EezPanel` или adapter слоя:

```cpp
class EezPanel {
private:
    bool showPageById(uint32_t pageId);
    bool setTextById(uint32_t elementId, const String& text);
    bool setColorById(uint32_t elementId, uint32_t color);
};
```

## Назначение файлов Panel

`Panel.h/.cpp` - публичный фасад экрана для остального проекта.

Он отвечает за:

- общий `init()` экрана;
- общий `process()` в главном loop;
- доступ к `ScreenModel`;
- показ страницы через модель;
- применение изменений элемента через модель.

`Boot`, `Page`, `State` и остальной код должны работать с экраном только через `Panel` и page-классы. Они не должны знать `lv_obj_t`, EEZ ids, GT911, ST7262, `smartdisplay_init()` и другие детали железа.

`EezPanel.h/.cpp` - низкоуровневый адаптер EEZ/LVGL.

Он отвечает за:

- вызов `ui_init()` и `ui_tick()` из generated EEZ-кода;
- показ страницы по generated page id;
- поиск `lv_obj_t*` по generated element id;
- установку текста, цвета, visible, value;
- привязку LVGL events к простому dispatch в `Panel`.

Generated ids (`scr_LOAD`, `txt_LOAD_VERSION`, `btn_MAIN_TASK`) допустимы здесь, но не в `Screen/Page`.

`DisplayDriver.h/.cpp` - простой слой инициализации дисплея JC8048W550C.

На первом этапе он должен использовать уже рабочий подход из `Screen32`: `esp32-smartdisplay`, board `JC8048W550C.json`, ST7262 parallel display, PSRAM framebuffer и подсветку. Не надо писать RGB-драйвер вручную, если готовый драйвер после копирования работает.

`TouchDriver.h/.cpp` - простой слой инициализации touch.

Для JC8048W550C это GT911 по I2C. Если `smartdisplay_init()` уже поднимает touch и регистрирует input device в LVGL, `TouchDriver::init()` может быть тонкой оберткой или временным no-op с `TODO`. Главное - не размазывать детали GT911 по `Panel` и страницам.

## Сборочные заглушки

Так как переходного Nextion backend не будет, нужны временные заглушки в новом `Screen`:

- если объект модели еще не связан с EEZ id, метод возвращает `false` и пишет лог;
- если страница еще не перенесена, `show()` показывает временную страницу или ничего не делает, но сборка не падает;
- старые методы `text()`, `showInfo()`, `setProgressColor()` остаются, но внутри работают через `Screen::Panel::model()`.

## Использование ScreenUI

В `C:\Users\sign\CODE\32\ScreenUI\` уже есть полезные части, но они должны быть скопированы внутрь `x32`:

- `eez_project/src/ui/*`;
- `generated/backend_pages/*_base.h`;
- `generated/shared/page_ids.generated.h`;
- `generated/shared/element_ids.generated.h`;
- `adapter/lvgl_eez/EezLvglAdapter.*`;
- `generated/frontend_meta/ui_object_map.generated.*`.

Их надо использовать как основу, но generated ids не должны протекать в `Screen/Page`.

Редактировать лучше исходный `.eez-project`, а generated-файлы воспринимать как результат генерации.

Не надо слепо переносить весь `Screen32` runtime:

- online/offline demo;
- Web target;
- frontend/backend protocol transport;
- UART/WebSocket transport runtime;
- тестовые и demo сценарии.

Для `x32` нужен локальный экранный фасад. Он может использовать идеи и фрагменты `Screen32`, но события должны идти напрямую в страницы и state machine текущего проекта:

```text
LVGL click
  -> EezPanel
  -> Panel dispatch
  -> Screen::main::onStart()
  -> App::state() / Scene
```

Если какой-то генератор Python нужен только для получения `page_ids`, `element_ids` и object-map, переносить нужно именно его минимальный рабочий набор. После переноса генераторы надо пересмотреть и убрать привязки к старой структуре соседнего проекта.

## Простота кода

Код `Screen` должен быть простым.

Допустимо сначала сделать более прямолинейно:

- один понятный `switch` по `elementId` вместо универсального dispatcher;
- явные поля в `ScreenModel` вместо сложной метапрограммной модели;
- простые методы `setText`, `setColor`, `setVisible` вместо обобщенного attribute runtime;
- временные заглушки с `TODO`, если страница еще не перенесена.

Не надо переносить в `x32` сложный protocol/runtime слой из `screenLIB`, если локальному проекту достаточно прямого вызова методов. Если часть кода взята из `Screen32` и выглядит слишком общей, ее нужно упростить до понятного варианта для текущего проекта.

## События

EEZ/LVGL actions должны только передавать событие в C++ слой:

```text
LVGL button click
  -> EEZ action
  -> Screen::Panel event dispatch
  -> named object in ScreenModel
  -> Screen::main::onStart()
  -> State/Scene
```

Бизнес-логика остается в `Screen/Page`, `Scene`, `State`.

## Критерий готовности

- Есть новая папка `src/Screen`.
- `Screen/Page` сохраняет текущую группировку страниц.
- Новые page-классы называются без префикса `p`: `load`, `main`, `info`, `error`.
- Публичный доступ к страницам короткий: `Screen::load`, `Screen::main`, `Screen::info`.
- `Screen/Page/Page.h` собирается без `Nextion.h`.
- Page-классы не используют id элементов.
- Есть объектная модель экрана с именованными страницами и элементами.
- `Screen/Panel` инициализирует EEZ/LVGL.
- `load` работает через `Screen::Panel::model()`.
- BOOT показывает статусы загрузки на JC8048W550C.
- Хотя бы одно событие кнопки доходит из LVGL/EEZ в C++.
- Неперенесенные страницы не ломают сборку.
