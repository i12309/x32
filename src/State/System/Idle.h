#pragma once
#include "State/State.h"
#if !defined(X32_TARGET_HEAD_UNIT)
#include "Machine/Context/IMachineContext.h"
#endif

class Idle : public State {
    public:
    Idle() : State(State::Type::IDLE) {}

    void oneRun() override {
        State::oneRun();
        Data::param.reset();
#if !defined(X32_TARGET_HEAD_UNIT)
        App::ctx().reg.reset(); 
#endif
        App::mode() = Mode::NORMAL;
        // Keep warnings until user acknowledges them in UI.
        App::diag().clearErrors();
    }

    State* run() override {return this;}
};
