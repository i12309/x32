#include "Screen/Page/Service/paper.h"

#include "App/App.h"

namespace Screen {

paper& paper::getInstance() {
    static paper page(App::panel());
    return page;
}

} // namespace Screen
