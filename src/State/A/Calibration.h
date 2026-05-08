#pragma once

#include "Service/Stats.h"
#include "Screen/Page/Main/Wait.h"
#include "State/A/CanActions.h"
#include "State/State.h"

class Calibration : public State {
public:
    Calibration() : State(State::Type::CALIBRATION) {}

    void oneRun() override {
        State::oneRun();

        PlanManager& plan = App::plan();
        if (plan.isActive()) return;

        Data::work.task.clear();
        Data::work.task.ID = 0;
        Data::work.task.MARK = 1;
        Data::work.task.OVER_mm = Data::tuning.OVER_mm;
        Data::work.task.PRODUCT_mm = 0;

        Data::work.TOTAL_CYCLES = 1;
        Data::work.TOTAL_CUTS = Data::tuning.CUT_count * 2;
        if (!Data::work.valid()) return;

        Data::work.print();

        plan.beginPlan(this->type());
        if (Data::param.productCutsCount == 1) {
            plan.addAction(State::Type::ACTION, &CanActions::TableUp, "TableUp");
        }

        plan.addAction(State::Type::ACTION, &CanActions::DetectPaper, "DetectPaper");
        plan.addAction(State::Type::ACTION, &CanActions::DetectMark, "DetectMark");
        plan.addAction(State::Type::ACTION, &Calibration::Feed, "Feed");
        plan.addAction(State::Type::ACTION, &CanActions::GuillotineCut, "CalibrationCut");
        plan.addAction(State::Type::ACTION, &Calibration::LoopCut, "LoopCut");

        plan.addAction(State::Type::ACTION, &Calibration::FeedForward, "FeedForward");
        plan.addAction(State::Type::ACTION, &CanActions::GuillotineCut, "ForwardCut");
        plan.addAction(State::Type::ACTION, &Calibration::FeedBackward, "FeedBackward");

        if (Data::param.productCutsCount == Data::tuning.CUT_count) {
            plan.addAction(State::Type::ACTION, &CanActions::EjectTail, "EjectTail");
        }

        plan.printPlan();
    }

    State* run() override {
        PlanManager& plan = App::plan();
        if (!plan.hasPending()) {
            Stats::getInstance().save();
            Screen::Wait::instance().back();
            return Factory(State::Type::SERVICE);
        }
        return Factory(plan.nextType(this->type()));
    }

private:
    static bool Feed() {
        Log::D(__func__);
        float mm = 0.0f;
        if (Data::param.cutsCount == 0) {
            mm = -15.0f + (Data::tuning.MARK_LENGHT_mm +
                           Data::tuning.OVER_mm +
                           Data::tuning.SENSOR_DISTANCE_mm +
                           Data::tuning.DELTA_mm);
        } else {
            mm = 15.0f;
        }
        Log::D("Расстояние: %f", mm);
        return CanActions::paperMoveMm(mm, true);
    }

    static bool LoopCut() {
        Log::D(__func__);
        Data::param.cutsCount++;
        Log::D("Рез: %d", Data::param.cutsCount);
        if (Data::param.cutsCount == 1) {
            PlanManager& plan = App::plan();
            plan.resetByActionName("Feed");
            plan.resetByActionName("CalibrationCut");
            plan.resetByActionName("LoopCut");
        }
        return true;
    }

    static bool FeedForward() {
        Log::D(__func__);
        const float mm = Data::tuning.DISTANCE_BETWEEN_MARKS_mm - Data::tuning.OVER_mm - 9.0f;
        Log::D("Расстояние: %f", mm);
        return CanActions::paperMoveMm(mm, true);
    }

    static bool FeedBackward() {
        Log::D(__func__);
        const float mm = -1.0f * (Data::tuning.SENSOR_DISTANCE_mm + 10.0f);
        Log::D("Расстояние: %f", mm);
        return CanActions::paperMoveMm(mm, true);
    }
};
