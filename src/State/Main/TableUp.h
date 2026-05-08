#pragma once
#include "State/State.h"

class TableUp : public State {
public:
    TableUp() : State(State::Type::TABLE_UP) {}

    void oneRun() override {
        State::oneRun();

    }

    State* run() override {


        return Factory(State::Type::DONE);
    }

};
