#pragma once

#include "Screen/Page/Page.h"

#include <Arduino.h>
#include <cstdint>
#include <vector>

namespace Screen {

class ShowStat : public Page {
public:
    static ShowStat& instance();

    void showDeviceMotors();
    void showTasks();
    void showProfiles();

protected:
    void onPrepare() override;

private:
    enum class ViewMode : uint8_t {
        DeviceMotors,
        Tasks,
        TaskDetail,
        TaskProfiles,
        TaskMotors,
        Profiles,
        ProfileDetailGlobal,
        ProfileDetailTask,
        ProfileMotorsGlobal,
        ProfileMotorsTask
    };

    enum class ActionType : uint8_t {
        None,
        OpenTaskDetail,
        OpenTaskProfiles,
        OpenTaskMotors,
        OpenProfileDetailGlobal,
        OpenProfileDetailTask,
        OpenProfileMotorsGlobal,
        OpenProfileMotorsTask
    };

    struct RowAction {
        RowAction() = default;
        RowAction(ActionType actionType, long actionTaskId = -1, long actionProfileId = -1)
            : type(actionType), taskId(actionTaskId), profileId(actionProfileId) {}

        ActionType type = ActionType::None;
        long taskId = -1;
        long profileId = -1;
    };

    struct Row {
        Row() = default;
        Row(const String& rowLabel, const String& rowValue)
            : label(rowLabel), value(rowValue), action() {}
        Row(const String& rowLabel, const String& rowValue, const RowAction& rowAction)
            : label(rowLabel), value(rowValue), action(rowAction) {}

        String label;
        String value;
        RowAction action;
    };

    struct ViewState {
        ViewMode view;
        long taskId;
        long profileId;
        uint8_t page;
    };

    static constexpr uint8_t kPageSize = 6;

    ShowStat() : Page(SCREEN_ID_STATS) {}

    void setView(ViewMode newView, long taskId, long profileId, bool push);
    void buildRows();
    void renderPage();
    void clearRows();
    void updateNextButton();
    void handleAction(const RowAction& action);
    int rowIndexFromEvent(lv_event_t* e) const;
    void nextPage();
    void goBack();

    static void popBack(lv_event_t* e);
    static void popNext(lv_event_t* e);
    static void popRow(lv_event_t* e);

    std::vector<Row> rows_;
    std::vector<ViewState> nav_;
    ViewMode view_ = ViewMode::DeviceMotors;
    long viewTaskId_ = -1;
    long viewProfileId_ = -1;
    uint8_t page_ = 0;
    uint8_t totalPages_ = 0;
    String title_;
    String field1_;
    String field2_;
};

}  // namespace Screen
