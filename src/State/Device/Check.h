#pragma once
#include "State/State.h"
#include "Scene/SceneManager.h"
#if !defined(X32_TARGET_HEAD_UNIT)
#include "Service/ESPUpdate.h"
#endif

class A_CHECK : public State {
public:
    A_CHECK() : State(State::Type::CHECK) {}

    void oneRun() override {
        State::oneRun();

        Data::param.checkFull = (getPreviousTypeState() == State::Type::BOOT);
        App::diag().clearErrors();
        App::diag().setCollecting(true);

        if (!Core::settings.CHECK_SYSTEM) return;

        taskId = App::sceneManager().check().all();
    }

    State* run() override {
        if (!Core::settings.CHECK_SYSTEM) return finish();

        SceneTaskStatus status = App::sceneManager().status(taskId);
        switch (status) {
            case SceneTaskStatus::Queued:
            case SceneTaskStatus::Running:
                return this;

            case SceneTaskStatus::Done:
                return finish();

            case SceneTaskStatus::Failed:
                return fail("CAN device self-test failed");

            case SceneTaskStatus::Timeout:
                return fail("CAN device self-test timeout");

            case SceneTaskStatus::Rejected:
                return fail("CAN device rejected self-test");

            case SceneTaskStatus::Unknown:
            default:
                return fail("CAN device self-test status is unknown");
        }
    }

private:
    SceneTaskId taskId = 0;

    State* finish() {
        Stats::getInstance().save();
        App::diag().setCollecting(false);

        State::Type returnType = getNextTypeState();
        setNexTypeState(State::Type::NULL_STATE);
        if (returnType == State::Type::NULL_STATE) returnType = State::Type::IDLE;
        if (App::diag().hasAny()) returnType = State::Type::ERROR;

#if !defined(X32_TARGET_HEAD_UNIT)
        ESPUpdate::getInstance().markCurrentFirmwareValid();
#endif
        return Factory(returnType);
    }

    State* fail(const String& message) {
        App::diag().addError(State::ErrorCode::CHECK_FAILED, message, "", false);
        App::diag().setCollecting(false);
        return Factory(State::Type::ERROR);
    }
};
