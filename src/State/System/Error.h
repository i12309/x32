#pragma once
#include "State/State.h"
#include "UI/Main/pERROR.h"
#include "Screen/Page/Main/Error.h"

class Error : public State {
public:

    Error() :State(State::Type::ERROR) {};

    void oneRun() override {
        State::oneRun();
        App::plan().clear();
        Screen::Error::instance().show();
        //pERROR::getInstance().show();

        // При ошибке останавливаем все моторы!
        App::ctx().reg.emergencyMotor();
        App::ctx().reg.emergencyClutch();
    }

    State* run() override {return this;}

    };
