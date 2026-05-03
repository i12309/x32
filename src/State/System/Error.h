#pragma once
#include "State/State.h"
#if !defined(X32_TARGET_HEAD_UNIT)
#include "Machine/Context/IMachineContext.h"
#include "UI/Main/pERROR.h"
#else
#include "Screen/Panel/Panel.h"
#endif

class Error : public State {
public:

    Error() :State(State::Type::ERROR) {};

    void oneRun() override {
        State::oneRun();
        App::plan().clear();
#if defined(X32_TARGET_HEAD_UNIT)
        App::panel().showError("Error", App::diag().currentOrNoError().description);
#else
        pERROR::getInstance().show();

        // При ошибке останавливаем все моторы!
        App::ctx().reg.emergencyMotor();
        App::ctx().reg.emergencyClutch();
#endif
    }

    State* run() override {return this;}

    };
