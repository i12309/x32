#pragma once

#include <cstdint>

#include "Service/Stats.h"
#include "Screen/Page/Main/Wait.h"
#include "Screen/Page/Service/Slice.h"
#include "State/Main/CanActions.h"
#include "State/State.h"

class Slice : public State {
public:
    Slice() : State(State::Type::SLICE) {}

    static void setSheetCount(int value) {
        requestedSheets() = value;
    }

    void oneRun() override {
        State::oneRun();

        PlanManager& plan = App::plan();
        if (plan.isActive()) return;

        measuredCount() = 0;
        totalLengthMm() = 0.0f;
        totalLengthSteps() = 0;
        failedFlag() = false;
        failedText() = "";

        if (requestedSheets() <= 0) {
            fail("Кол-во листов должно быть больше 0");
            return;
        }
        if (Data::work.profile.RATIO_mm <= 0.0f) {
            fail("Коэф. профиля (RATIO_mm) должен быть больше 0");
            return;
        }

        updateWaitProgress(0);

        plan.beginPlan(this->type());
        plan.addAction(State::Type::ACTION, &CanActions::PaperZeroPosition, "PaperZero");

        for (int i = 0; i < requestedSheets(); ++i) {
            plan.addAction(State::Type::ACTION, &CanActions::TableUp, "TableUp");
            plan.addAction(State::Type::ACTION, &CanActions::DetectPaper, "DetectFrontEdge");
            plan.addAction(State::Type::ACTION, &Slice::Edge, "Edge");
            plan.addAction(State::Type::ACTION, &CanActions::DetectMark, "DetectBackEdge");
            plan.addAction(State::Type::ACTION, &Slice::Calculate, "Calculate");
            plan.addAction(State::Type::ACTION, &CanActions::EjectTail, "EjectTail");
        }

        plan.printPlan();
    }

    State* run() override {
        if (failedFlag()) {
            return Factory(App::diag().add(State::ErrorCode::PAPER_NOT_FOUND,
                                           "Ошибка разделения листов",
                                           failedText()));
        }

        PlanManager& plan = App::plan();
        if (!plan.hasPending()) {
            if (Screen::Page::activePage() == &Screen::Wait::instance() && Screen::Page::previousPage() != nullptr) {
                Screen::Wait::instance().back();
            } else {
                Screen::Slice::instance().show();
            }

            setSlicePageResult();
            Stats::getInstance().save();
            return Factory(State::Type::SERVICE);
        }

        return Factory(plan.nextType(this->type()));
    }

private:
    static bool Edge() {
        return CanActions::PaperZeroPosition();
    }

    static bool Calculate() {
        Log::D("Data::param.markPosition = %d", Data::param.markPosition);
        if (Data::param.markPosition <= 0) {
            fail(String("Некорректная длина листа в шагах: ") + String(Data::param.markPosition));
            return true;
        }
        totalLengthMm() += static_cast<float>(Data::param.markPosition) / Data::work.profile.RATIO_mm;
        totalLengthSteps() += Data::param.markPosition;
        measuredCount()++;

        updateWaitProgress(measuredCount());
        return true;
    }

    static String makeSliceResultText() {
        return "ср " + String(averageLengthMm(), 2) + " мм, " + String(averageLengthSteps()) + " шагов";
    }

    static void updateWaitProgress(int currentSheet) {
        String statusText = "Лист " + String(currentSheet) + "/" + String(requestedSheets());
        Screen::Wait::instance().setTexts("", statusText, makeSliceResultText());
    }

    static void setSlicePageResult() {
        // TODO(ui-lvgl): add a named result field to SCREEN_ID_SLICE if this result must be visible there.
    }

    static float averageLengthMm() {
        if (measuredCount() <= 0) return 0.0f;
        return totalLengthMm() / measuredCount();
    }

    static int averageLengthSteps() {
        if (measuredCount() <= 0) return 0;
        return static_cast<int>(totalLengthSteps() / measuredCount());
    }

    static void fail(const String& text) {
        failedFlag() = true;
        failedText() = text;
        Log::E("Slice: %s", text.c_str());
    }

    static int& requestedSheets() {
        static int value = 0;
        return value;
    }

    static int& measuredCount() {
        static int value = 0;
        return value;
    }

    static float& totalLengthMm() {
        static float value = 0.0f;
        return value;
    }

    static int64_t& totalLengthSteps() {
        static int64_t value = 0;
        return value;
    }

    static bool& failedFlag() {
        static bool value = false;
        return value;
    }

    static String& failedText() {
        static String value = "";
        return value;
    }
};
