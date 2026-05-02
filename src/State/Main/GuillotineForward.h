#pragma once
#include "State/State.h"
#include "Scene/SceneManager.h"

class GuillotineForward : public State {
public:
    GuillotineForward() : State(State::Type::GUILLOTINE_FORWARD) {}

    void oneRun() override {        
        State::oneRun();
        taskId = App::sceneManager().guillotine().cut();
    }

    State* run() override {
        switch (App::sceneManager().status(taskId)) {
            case SceneTaskStatus::Queued:
            case SceneTaskStatus::Running:
                return this;

            case SceneTaskStatus::Done:
                return Factory(State::Type::DONE);

            case SceneTaskStatus::Timeout:
                return Factory(App::diag().addFatal(State::ErrorCode::GUILLOTINE_NOT_IN, "CAN guillotine task timeout"));

            case SceneTaskStatus::Rejected:
                return Factory(App::diag().addFatal(State::ErrorCode::GUILLOTINE, "CAN guillotine task rejected"));

            case SceneTaskStatus::Failed:
            case SceneTaskStatus::Unknown:
            default:
                return Factory(App::diag().addFatal(State::ErrorCode::GUILLOTINE, "CAN guillotine task failed"));
        }
    }

private:
    SceneTaskId taskId = 0;
};
