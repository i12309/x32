#include "Screen/Page/Task/task_run.h"

#include "App/App.h"

namespace Screen {

task_run& task_run::getInstance() {
    static task_run page(App::panel());
    return page;
}

} // namespace Screen
