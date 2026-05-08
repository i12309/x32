#pragma once

#include "Service/Stats.h"
#include "State/A/CanActions.h"
#include "State/State.h"

class A_PROCESS : public State {
public:
    A_PROCESS() : State(State::Type::PROCESS) {}

    void oneRun() override {
        State::oneRun();
        Data::param.startTime = micros();
        App::mode() = State::Mode::NORMAL;

        Stats::getInstance().jobStart(Data::work.task.ID, Data::work.profile.ID, Data::work.profile.RATIO_mm);

        PlanManager& plan = App::plan();
        plan.beginPlan(this->type());

        plan.addAction(State::Type::ACTION, &CanActions::TableUp, "TableUp");
        plan.addAction(State::Type::ACTION, &CanActions::DetectPaper, "DetectPaper");
        plan.addAction(State::Type::ACTION, &CanActions::DetectMark, "DetectMark");

        plan.addAction(State::Type::ACTION, &CanActions::ProductFeed, "ProductFeed");
        plan.addAction(State::Type::ACTION, &CanActions::GuillotineCut, "ProductCut");
        plan.addAction(State::Type::ACTION, &A_PROCESS::CutLoop, "CutLoop");

        for (int i = 0; i < Data::getFinishCUTs_count(); i++) {
            plan.addAction(State::Type::ACTION, &CanActions::TailFeed, "TailFeed");
            plan.addAction(State::Type::ACTION, &CanActions::GuillotineCut, "TailCut");
        }

        plan.addAction(State::Type::ACTION, &CanActions::EjectTail, "EjectTail");
        plan.addAction(State::Type::ACTION, &A_PROCESS::CycleLoop, "CycleLoop");

        plan.printPlan();
    }

    State* run() override {
        PlanManager& plan = App::plan();
        if (!plan.hasPending()) {
            Log::D("Время выполнения функции:  %f мин.",
                   (static_cast<float>(micros() - Data::param.startTime) / 60000000.0));
            Stats::getInstance().jobFinish();
            Data::param.reset();
            App::ctx().reg.reset();
            return Factory(State::Type::FINISH);
        }

        return Factory(plan.nextType(this->type()));
    }

private:
    static bool CutLoop() {
        if (Data::param.productCutsCount == Data::work.TOTAL_CUTS - 1 &&
            Data::param.cyclesCount < Data::work.TOTAL_CYCLES) {
            CanActions::TableUp();
        }

        Data::param.cutsCount++;
        if (Data::param.cutsCount >= 2 && (Data::param.cutsCount % 2 == 0)) {
            Data::param.productCutsCount++;
            Stats::getInstance().onProduct();
        }
        Log::D("Рез: %d, продуктовых: %d", Data::param.cutsCount, Data::param.productCutsCount);

        if (Data::param.productCutsCount < Data::work.TOTAL_CUTS) {
            PlanManager& plan = App::plan();
            plan.resetByActionName("ProductFeed");
            plan.resetByActionName("ProductCut");
            plan.resetByActionName("CutLoop");
        }
        return true;
    }

    static bool CycleLoop() {
        Data::param.cyclesCount++;
        Stats::getInstance().onSheet();

        if (Data::param.cyclesCount < Data::work.TOTAL_CYCLES) {
            PlanManager& plan = App::plan();

            plan.resetByActionName("TableUp");
            plan.resetByActionName("DetectPaper");
            plan.resetByActionName("DetectMark");
            plan.resetByActionName("ProductFeed");
            plan.resetByActionName("ProductCut");
            plan.resetByActionName("CutLoop");
            plan.resetByActionName("TailFeed");
            plan.resetByActionName("TailCut");
            plan.resetByActionName("EjectTail");
            plan.resetByActionName("CycleLoop");

            Log::D("Лист: %d из %d", Data::param.cyclesCount, Data::work.TOTAL_CYCLES);

            Data::param.cutsCount = 0;
            Data::param.productCutsCount = 0;
            Data::param.paperStepError = 0.0f;
        }
        return true;
    }
};
