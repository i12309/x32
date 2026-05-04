# Этап 02: runtime страниц и тонкие обертки объектов

## Цель

Сделать минимальный C++ слой, который позволяет писать страницы как бизнес-логику поверх EEZ/LVGL `objects`, без кода отрисовки и без генерации дополнительных фасадов.

## Работы

1. Создать `src/Screen/Page/Page.h|cpp`:
   - `screenId`;
   - `prepareOnce()`;
   - `show()`, `hide()`, `back()`;
   - `activePage`, `previousPage`;
   - static `process()` для вызова `activePage->onTick()`;
   - virtual `onPrepare()`, `onShow()`, `onHide()`, `onTick()`.
2. Создать `src/Screen/Panel/LvglHelpers.h` с маленьким `Ui` namespace:
   - `Ui::onPop(lv_obj_t* obj, lv_event_cb_t cb, void* userData = nullptr)`;
   - `Ui::onPush(lv_obj_t* obj, lv_event_cb_t cb, void* userData = nullptr)`;
   - `Ui::setText(...)`, `Ui::setBgColor(...)`, `Ui::setTextColor(...)`;
   - дополнительные helpers добавлять только по мере переноса страниц.
3. Для каждой страницы создать singleton-класс в `src/Screen/Page/<Name>.h|cpp`:
   - имена классов без суффикса `Page`: `Load`, `Main`, `TaskRun`, `Info`, `Input`;
   - доступ только через `<Name>::instance()`;
   - показ страницы: `<Name>::instance().show()`;
   - элементы страницы используются напрямую из EEZ generated `objects`: `objects.main_task`, `objects.load_version`.
4. Подключить `Page::process()` в `App::process()` рядом с `State::process()`.

`Pages.h|cpp`, `MainObjects`, wrapper-поля и генератор фасадов на этом этапе не создаются. Если позже прямые `objects.*` начнут мешать, можно добавить фасад только для реально сложных страниц.

## Пример целевого класса

```cpp
class Load : public Page {
public:
    static Load& instance() {
        static Load page;
        return page;
    }

    void onPrepare() override;
    void onShow() override;
    void onTick() override;

private:
    Load() : Page(SCREEN_ID_LOAD) {}
};
```

Внутри `Load` допустимы только обращения к `objects.*`, `Ui::*` helpers и сервисам. Создание `lv_label_create`, установка layout, размеров и стилей остается в EEZ.

## Базовый класс страницы

```cpp
class Page {
public:
    explicit Page(ScreensEnum screenId);

    void show();
    void hide();
    void back();

    static void process();

protected:
    virtual void onPrepare() {}
    virtual void onShow() {}
    virtual void onHide() {}
    virtual void onTick() {}

private:
    void prepareOnce();

    ScreensEnum screenId_;
    bool prepared_ = false;

    static Page* activePage_;
    static Page* previousPage_;
};
```

`show()` вызывает `prepareOnce()`, затем `loadScreen(screenId_)`, затем `onShow()`. `prepareOnce()` вызывает `onPrepare()` только один раз, чтобы callbacks не навешивались повторно при каждом показе страницы.

## Работа с событиями

События вешаются в `onPrepare()`, потому что он вызывается один раз за жизнь страницы. В `onShow()` callbacks не назначаются, иначе при каждом показе страницы появятся дубликаты.

Целевой стиль:

```cpp
void Main::onPrepare() {
    Ui::onPop(objects.main_task, Main::popTask);
    Ui::onPop(objects.main_service, Main::popService);
}
```

Логика остается внутри класса страницы:

```cpp
void Main::popTask(lv_event_t* e) {
    (void)e;
    Main::instance().openTask();
}

void Main::openTask() {
    TaskRun::instance().show();
}
```

`popTask` и `popService` должны быть `static`, потому что LVGL принимает callback вида `void (*)(lv_event_t*)`. Если нужна обычная нестатическая логика, static callback просто вызывает метод через `instance()`.

Минимальная форма `LvglHelpers.h`:

```cpp
namespace Ui {
    inline void onPop(lv_obj_t* obj, lv_event_cb_t cb, void* userData = nullptr) {
        lv_obj_add_event_cb(obj, cb, LV_EVENT_RELEASED, userData);
    }

    inline void onPush(lv_obj_t* obj, lv_event_cb_t cb, void* userData = nullptr) {
        lv_obj_add_event_cb(obj, cb, LV_EVENT_PRESSED, userData);
    }

    inline void setText(lv_obj_t* obj, const char* text) {
        lv_label_set_text(obj, text);
    }

    inline void setBgColor(lv_obj_t* obj, lv_color_t color) {
        lv_obj_set_style_bg_color(obj, color, LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    inline void setTextColor(lv_obj_t* obj, lv_color_t color) {
        lv_obj_set_style_text_color(obj, color, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
}
```

Для вложенного label без имени можно добавить helper:

```cpp
namespace Ui {
    lv_obj_t* firstChild(lv_obj_t* obj);
    lv_obj_t* firstLabel(lv_obj_t* obj);
}
```

Тогда текст кнопки `objects.main_task`, если label не назван в EEZ, меняется так:

```cpp
Ui::setText(Ui::firstLabel(objects.main_task), "Новое задание");
```

Если label часто меняется из бизнес-кода, лучше дать ему имя в EEZ, чтобы он появился в `objects` отдельным полем.

## Пример Main

`objects.main_task` создается в EEZ generated коде как `lv_button_create(...)`. При нажатии открываем `TaskRun`:

```cpp
class Main : public Page {
public:
    static Main& instance() {
        static Main page;
        return page;
    }

protected:
    void onPrepare() override {
        Ui::onPop(objects.main_task, Main::popTask);
        Ui::onPop(objects.main_service, Main::popService);
    }

private:
    Main() : Page(SCREEN_ID_MAIN) {}

    static void popTask(lv_event_t* e) {
        (void)e;
        TaskRun::instance().show();
    }

    static void popService(lv_event_t* e) {
        (void)e;
        Service::instance().show();
    }
};
```

## Проверки

- Можно показать страницу через `Main::instance().show()`.
- `back()` возвращает на предыдущую страницу.
- Повторный `show()` не навешивает duplicate callbacks.
- `onTick()` вызывается только у активной страницы.
- `Ui::onPop(objects.main_task, ...)` и `Ui::onPush(objects.main_task, ...)` вызывают нужные callbacks для LVGL-событий.
