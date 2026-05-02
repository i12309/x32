#include "Screen/Page/Main/main.h"

#include "App/App.h"

namespace Screen {

main& main::getInstance() {
    static main page(App::panel());
    return page;
}

} // namespace Screen
