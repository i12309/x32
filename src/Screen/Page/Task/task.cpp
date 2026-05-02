#include "Screen/Page/Task/task.h"

#include "App/App.h"

namespace Screen {

task& task::getInstance() {
    static task page(App::panel());
    return page;
}

} // namespace Screen
