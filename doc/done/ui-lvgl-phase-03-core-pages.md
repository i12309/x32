# Этап 03: базовые страницы `src/UI/Main`

## Цель

Перенести страницы из старой папки `src/UI/Main` на новый LVGL runtime из этапа 02.

На этом этапе нужно повторить страницы и бизнес-логику старых классов, но без Nextion, UART-команд, `PageManager` и отдельной логики обновления экрана. Страницы работают через `Screen::Page`, generated `objects.*` и маленькие helpers из `src/Screen/Panel/LvglHelpers.h`.

## Страницы

Перенести:

- `pLoad` -> `Screen::Load`, `SCREEN_ID_LOAD`;
- `pMain` -> `Screen::Main`, `SCREEN_ID_MAIN`;
- `pINFO` -> `Screen::Info`, `SCREEN_ID_INFO`;
- `pINPUT` -> `Screen::Input`, `SCREEN_ID_INPUT`;
- `pINIT` -> `Screen::INIT`, `SCREEN_ID_INIT`;
- `pWAIT` -> `Screen::Wait`, `SCREEN_ID_WAIT`;
- `pERROR` -> `Screen::Error`, `SCREEN_ID_INFO` или отдельная LVGL-страница, если она появится в EEZ.

Не переносить в этом этапе:

- `Keyboard`;
- generated actions `action_keyboard_text`, `action_keyboard_number`;
- ввод через LVGL keyboard/textarea.

Работу с `Keyboard` вынести в отдельный план, потому что там нужно отдельно согласовать UX ввода, callbacks и связь с `Input`.

## Общие правила

1. Новые классы этого этапа живут в `src/Screen/Page/Main/<Name>.h|cpp`.
2. Имена классов без суффикса `Page`: `Load`, `Main`, `Info`, `Input`, `INIT`, `Wait`, `Error`.
3. Доступ к странице только через singleton:

```cpp
Screen::Main::instance().show();
```

4. Наследование только от `Screen::Page`:

```cpp
namespace Screen {

class Main : public Page {
private:
    Main() : Page(SCREEN_ID_MAIN) {}
};

}  // namespace Screen
```

5. Переходы делать напрямую через страницы, без `PageManager`:

```cpp
Screen::TaskRun::instance().show();
Screen::Main::instance().show();
back();
```

6. События вешать в `onPrepare()` через `Ui::onPop(...)` / `Ui::onPush(...)`.
7. Тексты, цвета, видимость и значения менять через `Ui::*` helpers. Если helper-а нет, создать минимальный helper в `LvglHelpers.h`.
8. Не создавать wrapper-классы и фасады заранее. Работать прямо с `objects.*`.

## Удаляем из логики

Удалить или не переносить:

- `getHMIVersion()`;
- `VERSION_NEXTION`;
- `SetTFTVersion`;
- `UpdateTFT`;
- `Boot::InitNextion`;
- `ShowLoad` в Nextion-смысле;
- любые проверки и обновления `.tft`;
- `sendCommand`, `recvRetNumber`, `recvRetCommandFinished`;
- `NexText`, `NexButton`, `NexCheckbox`, `NexVariable`, `NexObject`, `nexLoop`.

Причина: backend и frontend теперь собираются как один firmware-монолит. У экрана больше нет отдельной HMI/TFT версии и отдельного процесса обновления.

Версию на `Load` показывать из общей firmware-версии приложения. MAC можно оставить, если он нужен пользователю.

## Load

Источник: `src/UI/Main/pLoad.h`.

Сделать:

- `Screen::Load : Screen::Page`;
- `Load::instance()`;
- `Load() : Page(SCREEN_ID_LOAD) {}`;
- `checkVersion()` заменить на заполнение общей версии firmware и MAC, если соответствующие объекты есть в EEZ.

Не переносить:

- `setStatus(...)`;
- `setProgressColor(...)`;
- `getProgressColor(...)`;
- `getHMIVersion()`;
- чтение `verHMI`;
- progress labels;
- статусный текст загрузки;
- Nextion-команды;
- сравнение версии HMI/TFT;
- обновление экрана.

Нужно найти в `objects` соответствия для:

- версии;
- MAC.

Если на новой EEZ-странице `Load` нет элементов для версии или MAC, не добавлять их в C++ только ради совместимости со старым Nextion UI.

## Main

Источник: `src/UI/Main/pMain.h|cpp`.

Сделать:

- `Screen::Main : Screen::Page`;
- `Main::instance()`;
- `Main() : Page(SCREEN_ID_MAIN) {}`;
- в `onPrepare()` повесить кнопки:

```cpp
Ui::onPop(objects.main_task, Main::popTask);
Ui::onPop(objects.main_profile, Main::popProfile);
Ui::onPop(objects.main_net, Main::popSettings);
Ui::onPop(objects.main_service, Main::popService);
Ui::onPop(objects.main_stats, Main::popStats);
Ui::onPop(objects.main_support, Main::popHelp);
```

- в `onShow()` перенести:
  - `Data::work.clear()`;
  - отображение Wi-Fi info/RSSI;
- в `onTick()` перенести периодическое обновление Wi-Fi каждые 5 секунд;
- проверку лицензии из старого `n_Idle()` оставить, если она нужна в текущем state flow;
- аппаратную кнопку `START` оставить через `App::ctx().reg.getButton("START")->isTrigger()`.

Переходы делать напрямую на новые страницы, например:

```cpp
Screen::TaskRun::instance().show();
Screen::Service::instance().show();
Screen::ProfileList::instance().show();
Screen::Wifi::instance().show();
Screen::Stats::instance().show();
Screen::Help::instance().show();
```

Если целевая страница еще не перенесена, временно закомментировать переход и оставить TODO. `PageManager` и legacy bridge не вводить.

## Info

Источник: `src/UI/Main/pINFO.h`.

Сделать:

- `Screen::Info : Screen::Page`;
- статический API, совместимый по смыслу:

```cpp
static void showInfo(
    const String& text1 = "",
    const String& text2 = "",
    const String& text3 = "",
    std::function<void()> onOk = nullptr,
    std::function<void()> onCancel = nullptr,
    bool showCancel = false,
    const String& okText = "",
    const String& cancelText = ""
);
```

- callbacks `OK` и `Cancel` через `Ui::onPop`;
- хранить `_okCallback`, `_cancelCallback`;
- скрывать/показывать cancel через `Ui::setHidden(...)`;
- текст кнопок менять только если в EEZ есть отдельные label-объекты или через `Ui::firstLabel(button)`.

После переноса постепенно заменить старые вызовы `pINFO::showInfo(...)` на `Screen::Info::showInfo(...)`. Временный facade не делать.

## Input

Источник: `src/UI/Main/pINPUT.h|cpp`.

Сделать только форму и бизнес-API без `Keyboard`:

- `Screen::Input : Screen::Page`;
- `showInput(...)`;
- тексты `title`, `info1`, `info2`, `defaultValue`;
- callbacks `OK` и `Cancel`;
- `autoBackOnClose`;
- чтение значения из LVGL text/label/textarea, который уже есть на странице.

Не делать:

- LVGL keyboard;
- generated keyboard actions;
- отдельный ввод текста/чисел через экранную клавиатуру.

Если текущая EEZ-страница `Input` без готового поля ввода неполная, оставить минимальный совместимый экран и зафиксировать TODO для отдельного Keyboard-плана.

## INIT

Источник: `src/UI/Main/pINIT.h|cpp`.

Сделать:

- `Screen::INIT : Screen::Page`;
- `prefill()`;
- выбор машины;
- сохранение `group`, `name`, `accessPoint`, `withTestData`;
- `buildConfig(...)`;
- `HTTP` flow:
  - спросить подключение к Wi-Fi через `Info`;
  - подключиться к default Wi-Fi или запросить вручную через `Input`;
  - запустить AP, если пользователь отказался от Wi-Fi;
  - показать IP через `Wait`.

Заменить Nextion combo helpers:

- `comboClear`;
- `comboSetList`;
- `comboGetIndex`;
- `comboSetIndex`.

На LVGL helpers:

- dropdown/list set options;
- dropdown/list get selected;
- checkbox get/set;
- textarea/label get/set.

Если нужных helpers нет, добавить их в `LvglHelpers.h` минимально, без больших wrapper-классов.

## Wait

Источник: `src/UI/Main/pWAIT.h`.

Сделать:

- `Screen::Wait : Screen::Page`;
- `wait(text1, text2, text3, time, callback, toBack = true)`;
- очистку текстов в `onShow()`;
- заполнение `wait_text1`, `wait_text2`, `wait_text3`.

Оставить старое поведение с `delay(time)` только если оно сейчас ожидаемо для boot/init flow. Если начнет мешать LVGL, заменить на неблокирующий timer отдельным изменением.

## Error

Источник: `src/UI/Main/pERROR.h|cpp`.

Сделать:

- `Screen::Error : Screen::Page`;
- `renderError()`;
- кнопки service/drop/next/back через `Ui::onPop`;
- `App::diag().resetCursor()` в `onShow()`;
- переходы на service/main через новые страницы. Если страницы еще не перенесены, закомментировать переход и оставить TODO.

Если в EEZ нет отдельной страницы `Error`, использовать `SCREEN_ID_INFO` как временное отображение диагностики через `Info`, либо добавить отдельную страницу в EEZ отдельной задачей.

## Helpers

В ходе переноса проверить, каких helpers не хватает в `src/Screen/Panel/LvglHelpers.h`.

Ожидаемо понадобятся:

- `Ui::setText(lv_obj_t*, const String&)`;
- `Ui::getText(lv_obj_t*) -> String`;
- `Ui::setBgColor(...)`;
- `Ui::setTextColor(...)`;
- `Ui::setHidden(...)`;
- `Ui::firstLabel(...)`;
- `Ui::setChecked(...)`, `Ui::isChecked(...)`;
- `Ui::dropdownSetOptions(...)`;
- `Ui::dropdownSelected(...)`;
- `Ui::dropdownSetSelected(...)`;
- `Ui::textareaSetText(...)`;
- `Ui::textareaText(...)`.

Добавлять helper только когда он реально нужен переносимой странице.

## Проверки

- `pio run -e JC8048W550C` проходит.
- Boot доходит до `Screen::Main`.
- `Load` не содержит старые progress/status методы; общая firmware-версия и MAC показываются только если такие объекты есть в EEZ.
- Кнопки `Main` вызывают нужные callbacks.
- Повторное открытие `Main` не создает duplicate callbacks.
- `Info::showInfo(...)` показывает тексты, OK и Cancel.
- `Input::showInput(...)` вызывает callbacks без Keyboard-логики.
- `INIT` сохраняет базовую конфигурацию и запускает HTTP/AP flow без Nextion-команд.
- `Wait` показывает тексты и возвращается назад при `toBack = true`.
- В коде новых страниц нет `Nex*`, `nexLoop`, `sendCommand`, `PageManager`, `getHMIVersion`, `UpdateTFT`.

## Риски

- Временные compatibility facade не делать. Если старый вызов еще некуда перенести, лучше временно закомментировать его с TODO.
- Некоторые EEZ-объекты имеют технические имена `objNN`. Если такой объект нужен в бизнес-логике, добавить TODO-комментарий и пропустить эту часть, чтобы не ломать сборку. Позже объект будет переименован в EEZ.
- `Input` пока без клавиатуры. Доделать ввод в отдельном Keyboard-плане.
