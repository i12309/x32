#include "Error.h"

#include "App/App.h"
#include "Catalog.h"
#include "Screen/Panel/LvglHelpers.h"
#include "Service/DeviceError.h"

#include <ui/screens.h>

namespace Screen {

Error& Error::instance() {
    static Error page;
    return page;
}

void Error::onShow() {
    App::diag().resetCursor();
    renderError();
}

void Error::renderError() {
    DeviceError& errors = App::diag();

    if (errors.hasMessages()) {
        const auto& err = errors.current();
        const auto& entry = errors.currentEntry();
        const size_t index = errors.currentIndex() + 1;
        const size_t total = errors.count();

        String title = "Ошибка";
        if (entry.kind == DeviceError::Kind::Warning) {
            title = "Предупреждение";
        } else if (entry.kind == DeviceError::Kind::Fatal) {
            title = "Критическая ошибка";
        }

        if (total > 1) {
            title += " (" + String(index) + " из " + String(total) + ")";
        }

        Ui::setText(objects.info_title, title);
        Ui::setText(objects.info_field1, Catalog::errorName(err.code));
        Ui::setText(objects.info_field2, err.value.length() > 0 ? err.value : err.description);
        Ui::setText(objects.info_field3, "");
    } else {
        const auto& err = errors.currentOrNoError();
        Ui::setText(objects.info_title, "Диагностика");
        Ui::setText(objects.info_field1, Catalog::errorName(err.code));
        Ui::setText(objects.info_field2, err.value);
        Ui::setText(objects.info_field3, "");
    }

    Ui::setText(objects.info_ok, "OK");
    Ui::setHidden(objects.info_cancel, true);
    // TODO(ui-lvgl): в EEZ нет отдельной страницы Error и отдельных кнопок next/back/service.
    // После добавления именованных объектов подключить callbacks через Ui::onPop().
}

}  // namespace Screen
