#include "ProfileList.h"

#include "Data.h"
#include "Screen/Page/Main/Main.h"
#include "Screen/Page/Profile/Profile.h"
#include "Screen/Page/Service/Calibration.h"
#include "Screen/Page/Service/Slice.h"
#include "Screen/Page/Task/Task.h"
#include "Screen/Page/Task/TaskRun.h"

namespace Screen {

ProfileList& ProfileList::instance() {
    static ProfileList page;
    return page;
}

void ProfileList::show() {
    display();
}

void ProfileList::display() {
    Data::profiles.countPages();

    List::ListRow rows[List::kMaxRows];
    int itemCount = 0;
    auto items = Data::profiles.getPage(itemCount);

    for (int i = 0; i < itemCount && i < static_cast<int>(List::kMaxRows); ++i) {
        rows[i].text = String("  ") + String(Data::profiles.curentPage * Data::profiles.pageSize + i + 1) + ". " + items[i].NAME;
        rows[i].checkVisible = false;
        rows[i].editVisible = false;
    }

    List::ListCallbacks callbacks;
    callbacks.onBack = ProfileList::popBack;
    callbacks.onAdd = ProfileList::popAdd;
    callbacks.onNext = ((Data::profiles.curentPage + 1) < Data::profiles.totalPages) ? ProfileList::popNext : nullptr;
    callbacks.onRow = ProfileList::popRow;

    List::instance().showList("Профили", rows, static_cast<size_t>(itemCount), callbacks);
}

void ProfileList::popBack(lv_event_t* e) {
    (void)e;
    ProfileList& page = instance();
    if (Data::profiles.curentPage > 0) {
        Data::profiles.curentPage--;
        page.display();
        return;
    }

    switch (page.backPageStatus_) {
        case Catalog::PageMode::pTaskRun: TaskRun::instance().show(); return;
        case Catalog::PageMode::pTask: Task::instance().show(); return;
        case Catalog::PageMode::pCalibration: Calibration::instance().show(); return;
        case Catalog::PageMode::pSlice: Slice::instance().show(); return;
        case Catalog::PageMode::pMain:
        default: Main::instance().show(); return;
    }
}

void ProfileList::popNext(lv_event_t* e) {
    (void)e;
    if ((Data::profiles.curentPage + 1) < Data::profiles.totalPages) {
        Data::profiles.curentPage++;
        instance().display();
    }
}

void ProfileList::popAdd(lv_event_t* e) {
    (void)e;
    Data::work.profile.clear();
    Profile& profile = Profile::instance();
    profile.setFormMode(Catalog::FormMode::CREATE);
    profile.show();
}

void ProfileList::popRow(lv_event_t* e) {
    int index = List::selectedIndex(e);
    if (index < 0) return;

    ProfileList& page = instance();
    Data::profiles.getByPage(index, Data::work.profile);
    if (!Data::work.profile.valid()) return;

    switch (page.backPageStatus_) {
        case Catalog::PageMode::pTaskRun:
            TaskRun::instance().show();
            return;
        case Catalog::PageMode::pTask:
            Task::instance().show();
            return;
        case Catalog::PageMode::pCalibration:
            Calibration::instance().show();
            return;
        case Catalog::PageMode::pSlice:
            Slice::instance().show();
            return;
        case Catalog::PageMode::pMain:
        default: {
            Profile& profile = Profile::instance();
            profile.setFormMode(Catalog::FormMode::EDIT);
            profile.show();
            return;
        }
    }
}

}  // namespace Screen
