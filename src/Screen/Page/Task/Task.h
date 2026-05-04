#pragma once

#include "Catalog.h"
#include "Screen/Page/Page.h"

namespace Screen {

class Task : public Page {
public:
    static Task& instance();

    void setFormMode(Catalog::FormMode mode) { formMode_ = mode; }

protected:
    void onPrepare() override;
    void onShow() override;

private:
    Task() : Page(SCREEN_ID_TASK) {}

    void applyFormMode();
    void fillFromWork();
    bool validateTaskInputs(const String& name, const String& product) const;
    void saveTask(bool create);

    static void popBack(lv_event_t* e);
    static void popSave(lv_event_t* e);
    static void popDelete(lv_event_t* e);
    static void popListProfile(lv_event_t* e);

    Catalog::FormMode formMode_ = Catalog::FormMode::VIEW;
};

}  // namespace Screen
