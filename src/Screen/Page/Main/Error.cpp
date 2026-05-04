#include "Error.h"

#include "App/App.h"
#include "Catalog.h"
#include "Screen/Page/Main/Main.h"
#include "Screen/Page/Service/Service.h"
#include "Screen/Panel/LvglHelpers.h"
#include "Service/DeviceError.h"
#include "State/State.h"

#include <ui/screens.h>

namespace Screen {

Error& Error::instance() {
    static Error page;
    return page;
}

void Error::onPrepare() {
    Ui::onPop(objects.info_back, Error::popBack);
    Ui::onPop(objects.info_next, Error::popNext);
    Ui::onPop(objects.info_cancel, Error::popService);
    Ui::onPop(objects.info_ok, Error::popReset);
}

void Error::onShow() {
    App::diag().resetCursor();
    renderError();
}

void Error::popBack(lv_event_t* e) {
    (void)e;
    if (Page::activePage() != &instance()) return;

    App::diag().prev();
    instance().renderError();
}

void Error::popNext(lv_event_t* e) {
    (void)e;
    if (Page::activePage() != &instance()) return;

    App::diag().next();
    instance().renderError();
}

void Error::popService(lv_event_t* e) {
    (void)e;
    if (Page::activePage() != &instance()) return;

    App::diag().clear();
    if (App::state() != nullptr) App::state()->setFactory(State::Type::SERVICE);
    Service::instance().show();
}

void Error::popReset(lv_event_t* e) {
    (void)e;
    if (Page::activePage() != &instance()) return;

    App::diag().clear();
    if (App::state() != nullptr) App::state()->setFactory(State::Type::IDLE);
    Main::instance().show();
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

    Ui::setText(objects.info_cancel, "Сервис");
    Ui::setText(objects.info_ok, "Сброс");
    Ui::setHidden(objects.info_cancel, false);
    Ui::setHidden(objects.info_back, !errors.canPrev());
    Ui::setHidden(objects.info_next, !errors.canNext());
}

}  // namespace Screen
