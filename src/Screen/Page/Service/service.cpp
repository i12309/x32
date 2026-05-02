#include "Screen/Page/Service/service.h"

#include "App/App.h"

namespace Screen {

service& service::getInstance() {
    static service page(App::panel());
    return page;
}

} // namespace Screen
