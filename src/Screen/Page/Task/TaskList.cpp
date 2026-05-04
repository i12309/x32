#include "TaskList.h"

#include "Data.h"
#include "Screen/Page/Task/Task.h"
#include "Screen/Page/Task/TaskRun.h"

namespace Screen {

TaskList& TaskList::instance() {
    static TaskList page;
    return page;
}

void TaskList::show() {
    display();
}

void TaskList::display() {
    Data::work.task.clear();
    Data::tasks.countPages();

    List::ListRow rows[List::kMaxRows];
    int itemCount = 0;
    auto items = Data::tasks.getPage(itemCount);

    for (int i = 0; i < itemCount && i < static_cast<int>(List::kMaxRows); ++i) {
        rows[i].text = String("  ") + String(Data::tasks.curentPage * Data::tasks.pageSize + i + 1) + ". " + items[i].NAME;
        rows[i].checkVisible = false;
        rows[i].editVisible = true;
    }

    List::ListCallbacks callbacks;
    callbacks.onBack = TaskList::popBack;
    callbacks.onAdd = TaskList::popAdd;
    callbacks.onNext = ((Data::tasks.curentPage + 1) < Data::tasks.totalPages) ? TaskList::popNext : nullptr;
    callbacks.onRow = TaskList::popRow;
    callbacks.onEdit = TaskList::popEdit;

    List::instance().showList("Задания", rows, static_cast<size_t>(itemCount), callbacks);
}

void TaskList::popBack(lv_event_t* e) {
    (void)e;
    if (Data::tasks.curentPage == 0) {
        TaskRun::instance().show();
        return;
    }
    Data::tasks.curentPage--;
    instance().display();
}

void TaskList::popNext(lv_event_t* e) {
    (void)e;
    if ((Data::tasks.curentPage + 1) < Data::tasks.totalPages) {
        Data::tasks.curentPage++;
        instance().display();
    }
}

void TaskList::popAdd(lv_event_t* e) {
    (void)e;
    Data::work.clear();
    Task& task = Task::instance();
    task.setFormMode(Catalog::FormMode::CREATE);
    task.show();
}

void TaskList::popRow(lv_event_t* e) {
    int index = List::selectedIndex(e);
    if (index < 0) return;

    Data::tasks.getByPage(index, Data::work.task);
    Data::profiles.getByID(Data::work.task.PROFILE_ID, Data::work.profile);
    TaskRun::instance().show();
}

void TaskList::popEdit(lv_event_t* e) {
    int index = List::selectedIndex(e);
    if (index < 0) return;

    Data::tasks.getByPage(index, Data::work.task);
    if (!Data::work.task.valid()) return;

    Data::profiles.getByID(Data::work.task.PROFILE_ID, Data::work.profile);
    Task& task = Task::instance();
    task.setFormMode(Catalog::FormMode::EDIT);
    task.show();
}

}  // namespace Screen
