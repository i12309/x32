#pragma once

#include "Service/Stats.h"
#include "Screen/Page/Main/Input.h"
#include "Screen/Page/Profile/Profile.h"
#include "State/Main/CanActions.h"
#include "State/State.h"

class Profilling : public State {
public:
    Profilling() : State(State::Type::PROFILING) {}

    void oneRun() override {
        State::oneRun();

        PlanManager& plan = App::plan();
        if (plan.isActive()) return;

        plan.beginPlan(this->type());
        plan.addAction(State::Type::ACTION, &CanActions::TableUp, "TableUp");
        plan.addAction(State::Type::ACTION, &CanActions::DetectPaper, "DetectPaper");
        plan.addAction(State::Type::ACTION, &Profilling::FeedFirst, "FeedFirst");
        plan.addAction(State::Type::ACTION, &CanActions::GuillotineCut, "FirstCut");

        for (int i = 0; i < Data::tuning.PROFILE_COUNT_CUT + 1; ++i) {
            plan.addAction(State::Type::ACTION, &Profilling::FeedNext, "FeedNext");
            plan.addAction(State::Type::ACTION, &CanActions::GuillotineCut, "ProfileCut");
        }

        plan.addAction(State::Type::ACTION, &Profilling::FeedOut, "FeedOut");
        plan.printPlan();
    }

    State* run() override {
        PlanManager& plan = App::plan();
        if (!plan.hasPending()) {
            ShowProfileInput();
            Stats::getInstance().save();
            return Factory(State::Type::FINISH);
        }
        return Factory(plan.nextType(this->type()));
    }

private:
    static bool FeedFirst() {
        Log::D(__func__);
        return CanActions::paperMoveMm(Data::tuning.SENSOR_DISTANCE_mm + Data::tuning.DELTA_mm, true);
    }

    static bool FeedNext() {
        Log::D(__func__);
        CanScenario& can = CanScenario::instance();
        if (can.paperMoveSteps(Data::tuning.PROFILE_WIDTH_step, true)) return true;
        App::diag().addFatal(State::ErrorCode::PAPER, "PROFILE paper move failed", can.lastError());
        return false;
    }

    static bool FeedOut() {
        Log::D(__func__);
        CanScenario& can = CanScenario::instance();
        if (can.paperMoveSteps(10000, false)) return true;
        App::diag().addFatal(State::ErrorCode::PAPER, "PROFILE feed out failed", can.lastError());
        return false;
    }

    static void ShowProfileInput() {
        Screen::Input::showInput(
            "Измерение ширины",
            "Померяйте ширину полосок (до сотых)",
            "Внесите получившееся значение (в мм)",
            "0",
            [](const String& widthText) {

                const float width = widthText.toFloat();
                if (width <= 0.0f) {
                    Log::D("Ввели значение <= 0");
                    return;
                }

                Data::work.profile.RATIO_mm = (Data::tuning.PROFILE_WIDTH_step / width);

                Screen::Profile::instance().show();
                Log::D(String(Data::work.profile.RATIO_mm, 3).c_str());
            },
            []() {
                Screen::Profile::instance().show();
            },
            2,
            false);
    }
};
