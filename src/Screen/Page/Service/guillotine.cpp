#include "Screen/Page/Service/guillotine.h"

#include "App/App.h"

namespace Screen {

guillotine& guillotine::getInstance() {
    static guillotine page(App::panel());
    return page;
}

} // namespace Screen
