#include "Screen/Page/Main/error.h"

#include "App/App.h"

namespace Screen {

error& error::getInstance() {
    static error page(App::panel());
    return page;
}

void error::showError(const String& title, const String& message) {
    show();
    panel_.showError(title, message);
}

} // namespace Screen
