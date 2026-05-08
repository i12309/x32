#pragma once
#include "State/State.h"
#include "Service/Stats.h"

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

        // Removed State/Main state call: TABLE_UP.
        // Removed State/Main state call: DETECT_PAPER.
        // Removed State/Main state call: DETECT_MARK.

        // цикл листа 
        // Removed State/Main state call: PAPER_PULL.
        // Removed State/Main state call: GUILLOTINE_FORWARD.
        plan.addAction(State::Type::ACTION, &A_PROCESS::CutLoop, "CutLoop");

        // вычисляем сколько в конце надо будет сделать финишных резов что бы хвост бумаги нашинковать 
        for(int i = 0; i < Data::getFinishCUTs_count(); i++) {
            //plan.addAction(State::Type::ACTION, &A_PROCESS::FeedEnd, "FeedEnd");
            // Removed State/Main state call: PAPER_MOVE.
            // Removed State/Main state call: GUILLOTINE_FORWARD.
        }

        // Removed State/Main state call: PAPER_MOVE.

        // цикл процесса
        plan.addAction(State::Type::ACTION, &A_PROCESS::CycleLoop, "CycleLoop");

        plan.printPlan();
    }

    State* run() override {
        PlanManager& plan = App::plan();
        if (!plan.hasPending()) {
            Log::D("Время выполнения функции:  %f мин.",(static_cast<float>(micros() - Data::param.startTime) / 60000000.0));
            Stats::getInstance().jobFinish();
            Data::param.reset();
            App::ctx().reg.reset();
            return Factory(State::Type::FINISH);
        }

        // Это для приладки. Остановка после каждого реза продукта  
        // Removed State/Main state check: GUILLOTINE_FORWARD.
        return Factory(plan.nextType(this->type()));
    }

private:

    //static bool FeedEnd() {App::ctx().mPaper->moveMMAccum(15, Data::work.profile.RATIO_mm, true);return true;}

    static bool CutLoop() {

        // перед последним продуктовым резом листа поднять стол  
        if (Data::param.productCutsCount == Data::work.TOTAL_CUTS - 1 && Data::param.cyclesCount < Data::work.TOTAL_CYCLES){
            // Removed Scene method call: tableUp(Catalog::SPEED::Normal).
        }
        
        Data::param.cutsCount++; // прибавляем рез
        if (Data::param.cutsCount >= 2 && (Data::param.cutsCount % 2 == 0)) {
            Data::param.productCutsCount++; // Продуктовая визитка формируется каждым вторым резом: 2,4,6,...
            Stats::getInstance().onProduct();
        }
        Log::D("Рез: %d, продуктовых: %d", Data::param.cutsCount, Data::param.productCutsCount);

        if (Data::param.productCutsCount < Data::work.TOTAL_CUTS) {
            PlanManager& plan = App::plan();
            // Removed State/Main state reset: PAPER_PULL.
            // Removed State/Main state reset: GUILLOTINE_FORWARD.
            plan.resetByActionName("CutLoop"); // сбрасываем текущий ACTION что бы в след раз еще раз запустить 
        }
        return true;
    }

    static bool CycleLoop() {
        Data::param.cyclesCount++; // считаем листы
        Stats::getInstance().onSheet();

        if (Data::param.cyclesCount < Data::work.TOTAL_CYCLES) {
            PlanManager& plan = App::plan();

            // Removed State/Main state reset: DETECT_PAPER.
            // Removed State/Main state reset: DETECT_MARK.
            // Removed State/Main state reset: PAPER_PULL.
            // Removed State/Main state reset: GUILLOTINE_FORWARD.
            plan.resetByActionName("CutLoop");
            plan.resetByActionName("FeedEnd");
            // Removed State/Main state reset: PAPER_MOVE.
            plan.resetByActionName("CycleLoop");// сбрасываем текущий ACTION что бы в след раз еще раз запустить
            
            Log::D("Лист: %d из %d",Data::param.cyclesCount, Data::work.TOTAL_CYCLES); 

            Data::param.cutsCount = 0;
            Data::param.productCutsCount = 0;
            Data::param.paperStepError = 0.0f;
        }
        return true;
    }
};
