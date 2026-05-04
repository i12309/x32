#include "Calibration.h"

#include "Catalog.h"
#include "Data.h"
#include "Screen/Page/Profile/ProfileList.h"
#include "Screen/Page/Service/Service.h"
#include "Screen/Panel/LvglHelpers.h"

#include <ui/screens.h>

namespace Screen {

Calibration& Calibration::instance() {
    static Calibration page;
    return page;
}

void Calibration::onPrepare() {
    Ui::onPop(objects.calibration_back, Calibration::popBack);
    Ui::onPop(objects.calibration_save, Calibration::popSave);
    Ui::onPop(objects.calibration_list_profile, Calibration::popProfileList);
}

void Calibration::onShow() {
    Ui::setText(objects.calibration_list_profile,
                Data::work.profile.valid() ? Data::work.profile.NAME : "Выберите профиль");
    // TODO(ui-lvgl): finish calibration numeric fields after EEZ object names are stabilized.
}

void Calibration::popBack(lv_event_t* e) {
    (void)e;
    Service::instance().show();
}

void Calibration::popProfileList(lv_event_t* e) {
    (void)e;
    ProfileList::instance().setBackPage(Catalog::PageMode::pCalibration);
    ProfileList::instance().show();
}

void Calibration::popSave(lv_event_t* e) {
    (void)e;
    Data::tuning.save();
    Service::instance().show();
}

}  // namespace Screen
