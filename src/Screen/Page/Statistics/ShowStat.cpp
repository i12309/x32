#include "ShowStat.h"

#include "Data.h"
#include "Screen/Page/Statistics/Statistics.h"
#include "Screen/Panel/LvglHelpers.h"
#include "Service/Stats.h"

#include <ui/screens.h>

namespace {

String formatCount(uint64_t value, const char* unit) {
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%llu", static_cast<unsigned long long>(value));
    String result(buffer);
    if (unit != nullptr && unit[0] != '\0') {
        result += " ";
        result += unit;
    }
    return result;
}

String formatMeters(uint64_t um) {
    String result(static_cast<float>(um) / 1000000.0f, 2);
    result += " м";
    return result;
}

String getTaskName(long id) {
    String name;
    if (Data::tasks.getNameByID(id, name)) return name;
    return String("ID ") + String(id);
}

String getProfileName(long id) {
    String name;
    if (Data::profiles.getNameByID(id, name)) return name;
    return String("ID ") + String(id);
}

lv_obj_t* paramObject(uint8_t index) {
    switch (index) {
        case 0: return objects.stats_param1;
        case 1: return objects.stats_param2;
        case 2: return objects.stats_param3;
        case 3: return objects.stats_param4;
        case 4: return objects.stats_param5;
        case 5: return objects.stats_param6;
        default: return nullptr;
    }
}

lv_obj_t* valueObject(uint8_t index) {
    switch (index) {
        case 0: return objects.stats_value1;
        case 1: return objects.stats_value2;
        case 2: return objects.stats_value3;
        case 3: return objects.stats_value4;
        case 4: return objects.stats_value5;
        case 5: return objects.stats_value6;
        default: return nullptr;
    }
}

}  // namespace

namespace Screen {

ShowStat& ShowStat::instance() {
    static ShowStat page;
    return page;
}

void ShowStat::showDeviceMotors() {
    nav_.clear();
    setView(ViewMode::DeviceMotors, -1, -1, false);
}

void ShowStat::showTasks() {
    nav_.clear();
    setView(ViewMode::Tasks, -1, -1, false);
}

void ShowStat::showProfiles() {
    nav_.clear();
    setView(ViewMode::Profiles, -1, -1, false);
}

void ShowStat::onPrepare() {
    Ui::onPop(objects.stats_back, ShowStat::popBack);
    Ui::onPop(objects.stats_next, ShowStat::popNext);
    for (uint8_t i = 0; i < kPageSize; ++i) {
        void* userData = reinterpret_cast<void*>(i);
        Ui::onPop(paramObject(i), ShowStat::popRow, userData);
        Ui::onPop(valueObject(i), ShowStat::popRow, userData);
    }
}

void ShowStat::setView(ViewMode newView, long taskId, long profileId, bool push) {
    if (push) nav_.push_back({view_, viewTaskId_, viewProfileId_, page_});

    view_ = newView;
    viewTaskId_ = taskId;
    viewProfileId_ = profileId;
    page_ = 0;

    buildRows();
    renderPage();
}

void ShowStat::buildRows() {
    rows_.clear();
    Stats& stats = Stats::getInstance();

    switch (view_) {
        case ViewMode::DeviceMotors: {
            title_ = "Моторы";
            field1_ = "Мотор";
            field2_ = "Пробег";
            for (const auto& motor : stats.getDeviceMotors()) {
                rows_.push_back({motor.name, formatCount(motor.steps, "шаг"), {}});
            }
            break;
        }
        case ViewMode::Tasks: {
            title_ = "Задания";
            field1_ = "Задание";
            field2_ = "Запусков";
            for (const auto& task : stats.getTaskRuns()) {
                Row row;
                row.label = getTaskName(task.id);
                row.value = formatCount(task.runs, "кол-во");
                row.action = RowAction(ActionType::OpenTaskDetail, task.id, -1);
                rows_.push_back(row);
            }
            break;
        }
        case ViewMode::Profiles: {
            title_ = "Профили";
            field1_ = "Профиль";
            field2_ = "Запусков";
            for (const auto& profile : stats.getProfileRuns()) {
                Row row;
                row.label = getProfileName(profile.id);
                row.value = formatCount(profile.runs, "кол-во");
                row.action = RowAction(ActionType::OpenProfileDetailGlobal, -1, profile.id);
                rows_.push_back(row);
            }
            break;
        }
        case ViewMode::TaskDetail: {
            title_ = getTaskName(viewTaskId_);
            field1_ = "Метрика";
            field2_ = "Значение";
            Stats::AggregateStat agg;
            stats.getTaskAggregate(viewTaskId_, agg);
            rows_.push_back({"Запусков", formatCount(agg.runs, ""), {}});
            rows_.push_back({"Продуктов", formatCount(agg.products, "шт"), {}});
            rows_.push_back({"Листов", formatCount(agg.sheets, "шт"), {}});
            rows_.push_back({"Длина бумаги", formatMeters(agg.paper_um), {}});
            rows_.push_back({"Отказов", formatCount(agg.rejects, ""), {}});
            rows_.push_back(Row("Профилей", formatCount(agg.profiles, "кол-во"), RowAction(ActionType::OpenTaskProfiles, viewTaskId_, -1)));
            rows_.push_back(Row("Моторы", "Показать", RowAction(ActionType::OpenTaskMotors, viewTaskId_, -1)));
            break;
        }
        case ViewMode::TaskProfiles: {
            title_ = getTaskName(viewTaskId_);
            field1_ = "Профиль";
            field2_ = "Запусков";
            for (const auto& profile : stats.getTaskProfileRuns(viewTaskId_)) {
                rows_.push_back(Row(getProfileName(profile.id),
                                    formatCount(profile.runs, "кол-во"),
                                    RowAction(ActionType::OpenProfileDetailTask, viewTaskId_, profile.id)));
            }
            break;
        }
        case ViewMode::TaskMotors:
        case ViewMode::ProfileMotorsGlobal:
        case ViewMode::ProfileMotorsTask: {
            title_ = "Моторы";
            field1_ = "Мотор";
            field2_ = "Пробег";
            Stats::AggregateStat agg;
            if (view_ == ViewMode::TaskMotors) stats.getTaskAggregate(viewTaskId_, agg);
            if (view_ == ViewMode::ProfileMotorsGlobal) stats.getProfileAggregate(viewProfileId_, agg);
            if (view_ == ViewMode::ProfileMotorsTask) stats.getTaskProfileAggregate(viewTaskId_, viewProfileId_, agg);
            for (const auto& motor : agg.motors) {
                rows_.push_back({motor.name, formatCount(motor.steps, "шаг"), {}});
            }
            break;
        }
        case ViewMode::ProfileDetailGlobal:
        case ViewMode::ProfileDetailTask: {
            title_ = getProfileName(viewProfileId_);
            field1_ = "Метрика";
            field2_ = "Значение";
            Stats::AggregateStat agg;
            if (view_ == ViewMode::ProfileDetailGlobal) stats.getProfileAggregate(viewProfileId_, agg);
            else stats.getTaskProfileAggregate(viewTaskId_, viewProfileId_, agg);
            rows_.push_back({"Запусков", formatCount(agg.runs, ""), {}});
            rows_.push_back({"Продуктов", formatCount(agg.products, "шт"), {}});
            rows_.push_back({"Листов", formatCount(agg.sheets, "шт"), {}});
            rows_.push_back({"Длина бумаги", formatMeters(agg.paper_um), {}});
            rows_.push_back({"Отказов", formatCount(agg.rejects, ""), {}});
            rows_.push_back(Row("Моторы",
                                "Показать",
                                RowAction(view_ == ViewMode::ProfileDetailGlobal ? ActionType::OpenProfileMotorsGlobal : ActionType::OpenProfileMotorsTask,
                                          viewTaskId_,
                                          viewProfileId_)));
            break;
        }
    }
}

void ShowStat::renderPage() {
    show();
    Ui::setText(objects.stats_title, title_);
    Ui::setText(objects.stats_field1, field1_);
    Ui::setText(objects.stats_field2, field2_);
    clearRows();

    totalPages_ = rows_.empty() ? 1 : static_cast<uint8_t>((rows_.size() + kPageSize - 1) / kPageSize);
    if (page_ >= totalPages_) page_ = totalPages_ - 1;

    size_t start = static_cast<size_t>(page_) * kPageSize;
    for (uint8_t i = 0; i < kPageSize; ++i) {
        size_t index = start + i;
        if (index >= rows_.size()) break;
        Ui::setText(paramObject(i), rows_[index].label);
        Ui::setText(valueObject(i), rows_[index].value);
        if (rows_[index].action.type != ActionType::None) {
            Ui::setBgColor(valueObject(i), lv_color_hex(0xd74b21));
        }
    }

    updateNextButton();
}

void ShowStat::clearRows() {
    for (uint8_t i = 0; i < kPageSize; ++i) {
        Ui::setText(paramObject(i), "");
        Ui::setText(valueObject(i), "");
        Ui::setBgColor(valueObject(i), lv_color_hex(0xffffff));
    }
}

void ShowStat::updateNextButton() {
    Ui::setHidden(objects.stats_next, rows_.empty() || page_ + 1 >= totalPages_);
}

void ShowStat::handleAction(const RowAction& action) {
    switch (action.type) {
        case ActionType::OpenTaskDetail: setView(ViewMode::TaskDetail, action.taskId, -1, true); return;
        case ActionType::OpenTaskProfiles: setView(ViewMode::TaskProfiles, action.taskId, -1, true); return;
        case ActionType::OpenTaskMotors: setView(ViewMode::TaskMotors, action.taskId, -1, true); return;
        case ActionType::OpenProfileDetailGlobal: setView(ViewMode::ProfileDetailGlobal, -1, action.profileId, true); return;
        case ActionType::OpenProfileDetailTask: setView(ViewMode::ProfileDetailTask, action.taskId, action.profileId, true); return;
        case ActionType::OpenProfileMotorsGlobal: setView(ViewMode::ProfileMotorsGlobal, -1, action.profileId, true); return;
        case ActionType::OpenProfileMotorsTask: setView(ViewMode::ProfileMotorsTask, action.taskId, action.profileId, true); return;
        case ActionType::None: return;
    }
}

int ShowStat::rowIndexFromEvent(lv_event_t* e) const {
    return static_cast<int>(reinterpret_cast<uintptr_t>(lv_event_get_user_data(e)));
}

void ShowStat::nextPage() {
    if (page_ + 1 < totalPages_) {
        page_++;
        renderPage();
    }
}

void ShowStat::goBack() {
    if (page_ > 0) {
        page_--;
        renderPage();
        return;
    }
    if (!nav_.empty()) {
        ViewState state = nav_.back();
        nav_.pop_back();
        view_ = state.view;
        viewTaskId_ = state.taskId;
        viewProfileId_ = state.profileId;
        page_ = state.page;
        buildRows();
        renderPage();
        return;
    }
    Statistics::instance().show();
}

void ShowStat::popBack(lv_event_t* e) {
    (void)e;
    instance().goBack();
}

void ShowStat::popNext(lv_event_t* e) {
    (void)e;
    instance().nextPage();
}

void ShowStat::popRow(lv_event_t* e) {
    ShowStat& page = instance();
    int localIndex = page.rowIndexFromEvent(e);
    if (localIndex < 0) return;
    size_t index = static_cast<size_t>(page.page_) * kPageSize + static_cast<size_t>(localIndex);
    if (index >= page.rows_.size()) return;
    page.handleAction(page.rows_[index].action);
}

}  // namespace Screen
