#pragma once

#include "State/State.h"
#include "Scene/SceneManager.h"

class PaperMove : public State {
public:
    PaperMove() : State(State::Type::PAPER_MOVE) {}

    // Одноразовый запуск шага перемещения бумаги по параметрам из плана.
    // Приоритет параметров:
    // 1) steps (если задан)
    // 2) mm -> конвертация в steps через RATIO_mm
    void oneRun() override {
        State::oneRun();

        const Catalog::WorkParam params = App::plan().getCurrentParams();

        direction = params.hasDir ? params.dir : Catalog::DIR::Forward;
        speed = params.hasSpeed ? params.speed : Catalog::SPEED::Normal;

        bool hasDistance = false;
        float distanceMm = 0.0f;

        if (params.hasSteps) {
            hasDistance = true;
            int32_t steps = params.steps;
            // Если направление явно не задано, берем его из знака шага.
            if (!params.hasDir && steps < 0) direction = Catalog::DIR::Backward;
            if (steps < 0) steps = -steps;

            if (Data::work.profile.RATIO_mm <= 0.0f) {
                errorText = "PAPER_MOVE: невозможно конвертировать steps в mm, RATIO_mm не задан";
                failed = true;
                return;
            }
            distanceMm = static_cast<float>(steps) / Data::work.profile.RATIO_mm;
        } else if (params.hasMm) {
            hasDistance = true;
            distanceMm = params.mm;
            // Если направление явно не задано, берем его из знака расстояния.
            if (!params.hasDir && distanceMm < 0.0f) direction = Catalog::DIR::Backward;
            if (distanceMm < 0.0f) distanceMm = -distanceMm;
        }

        if (!hasDistance) {
            errorText = "PAPER_MOVE: не задано расстояние (steps или mm)";
            failed = true;
            return;
        }

        requestedMm = (direction == Catalog::DIR::Backward) ? -distanceMm : distanceMm;

        if (requestedMm == 0.0f) {
            started = true;
            return;
        }

        taskId = App::sceneManager().paper().feed(requestedMm, speedMmS(speed));
        started = true;
    }

    // Ожидание завершения scene-task на CAN-device.
    State* run() override {
        if (failed) {
            return Factory(App::diag().addError(State::ErrorCode::PAPER, errorText, "", false));
        }

        if (!started) {
            return Factory(App::diag().addError(State::ErrorCode::UNKNOWN_ERROR, "PAPER_MOVE: шаг не был запущен в oneRun", "", false));
        }

        if (requestedMm == 0.0f) return Factory(State::Type::DONE);

        switch (App::sceneManager().status(taskId)) {
            case SceneTaskStatus::Queued:
            case SceneTaskStatus::Running:
                return this;

            case SceneTaskStatus::Done:
                return Factory(State::Type::DONE);

            case SceneTaskStatus::Timeout:
                return Factory(App::diag().addError(State::ErrorCode::PAPER_SEARCH, "CAN paper task timeout", "", false));

            case SceneTaskStatus::Rejected:
                return Factory(App::diag().addError(State::ErrorCode::PAPER, "CAN paper task rejected", "", false));

            case SceneTaskStatus::Failed:
            case SceneTaskStatus::Unknown:
            default:
                return Factory(App::diag().addError(State::ErrorCode::PAPER, "CAN paper task failed", "", false));
        }
    }

private:
    static uint16_t speedMmS(Catalog::SPEED speed) {
        (void)speed;
        return 0;
    }

    SceneTaskId taskId = 0;
    Catalog::DIR direction = Catalog::DIR::Forward;
    Catalog::SPEED speed = Catalog::SPEED::Normal;
    bool started = false;
    bool failed = false;
    float requestedMm = 0.0f;
    String errorText = "";
};
