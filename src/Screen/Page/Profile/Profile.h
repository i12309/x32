#pragma once

#include "Catalog.h"
#include "Screen/Page/Page.h"

namespace Screen {

class Profile : public Page {
public:
    static Profile& instance();

    void setFormMode(Catalog::FormMode mode) { formMode_ = mode; }

protected:
    void onPrepare() override;
    void onShow() override;
    void onTick() override;

private:
    Profile() : Page(SCREEN_ID_PROFILE) {}

    void applyFormMode();
    void fillFromWork();
    void saveProfile(bool create);

    static void popBack(lv_event_t* e);
    static void popSave(lv_event_t* e);
    static void popDelete(lv_event_t* e);
    static void popProfile(lv_event_t* e);

    Catalog::FormMode formMode_ = Catalog::FormMode::VIEW;
};

}  // namespace Screen
