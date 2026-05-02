#pragma once
#include "State/State.h"
#include "Scene/SceneManager.h"

class TableUp : public State {
public:
    TableUp() : State(State::Type::TABLE_UP) {}

    void oneRun() override {
        State::oneRun();
        taskId = App::sceneManager().table().up();
    }

    State* run() override {
        switch (App::sceneManager().status(taskId)) {
            case SceneTaskStatus::Queued:
            case SceneTaskStatus::Running:
                return this;

            case SceneTaskStatus::Done:
                return Factory(State::Type::DONE);

            case SceneTaskStatus::Timeout:
                return Factory(App::diag().addFatal(State::ErrorCode::TABLE_NOT_UP, "CAN table task timeout"));

            case SceneTaskStatus::Rejected:
                return Factory(App::diag().addFatal(State::ErrorCode::TABLE, "CAN table task rejected"));

            case SceneTaskStatus::Failed:
            case SceneTaskStatus::Unknown:
            default:
                return Factory(App::diag().addFatal(State::ErrorCode::TABLE, "CAN table task failed"));
        }
    }

private:
    SceneTaskId taskId = 0;
};
