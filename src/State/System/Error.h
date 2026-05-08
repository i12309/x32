#pragma once
#include "State/State.h"
#include "Screen/Page/Main/Error.h"
#include "Remote/CanMachine.h"

class Error : public State {
public:

    Error() :State(State::Type::ERROR) {};

    void oneRun() override {
        State::oneRun();
        App::plan().clear();
        Screen::Error::instance().show();
        // При ошибке останавливаем все моторы!
        Remote::CanMachine::instance().stopAll();
    }

    State* run() override {return this;}

    };
