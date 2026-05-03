# Этап 02: runtime страниц и тонкие обертки объектов

## Цель

Сделать C++ слой, который позволяет писать страницы как бизнес-логику поверх `objects`, без кода отрисовки.

## Работы

1. Создать `src/Screen/Page/IPage.h|cpp`:
   - имя страницы;
   - `screenId`;
   - `prepareOnce()`;
   - virtual `onPrepare()`, `onShow()`, `onHide()`, `onTick()`.
2. Создать `src/Screen/Page/PageManager.h|cpp`:
   - регистрация страниц;
   - `show(ScreensEnum id)` и `show(const char* name)`;
   - `activePage`, `previousPage`, `back()`;
   - вызов `loadScreen(screenId)`, затем `onShow()`;
   - вызов `activePage->onTick()` из общего process.
3. Создать `src/Screen/Panel/LvglWidgets.h`:
   - `TextRef` для `lv_label_set_text`, `lv_label_get_text`;
   - `ButtonRef` для `lv_obj_add_event_cb`;
   - `CheckboxRef`, `DropdownRef`, `TextareaRef` по мере миграции страниц;
   - единый callback bridge `lv_event_t -> std::function` или статические handlers без heap-рисков.
4. Создать `src/Screen/Page/Pages.h|cpp`:
   - единая фабрика/регистрация singleton-страниц;
   - typed getters для часто используемых страниц: `Load`, `Main`, `TaskRun`, `Info`, `Input`.
5. Подключить `PageManager::tick()` в `App::process()` рядом с `State::process()`.

## Пример целевого класса

```cpp
class Load : public IPage {
public:
    Load() : IPage("Load", SCREEN_ID_LOAD) {}

    void onPrepare() override;
    void onShow() override;
    void onTick() override;

    void setStatus(const String& text);
    void setProgressColor(int index, lv_color_t color);
};
```

Внутри `Load` допустимы только обращения к wrapper-объектам и сервисам. Создание `lv_label_create`, установка layout, размеров и стилей остается в EEZ.

## Проверки

- Можно показать страницу по enum и по имени.
- `back()` возвращает на предыдущую страницу.
- Повторный `show()` не навешивает duplicate callbacks.
- `onTick()` вызывается только у активной страницы.
