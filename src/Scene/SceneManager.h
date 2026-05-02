#pragma once

#include "Scene/CheckScene.h"
#include "Scene/EmergencyScene.h"
#include "Scene/GuillotineScene.h"
#include "Scene/PaperScene.h"
#include "Scene/TableScene.h"

class SceneManager {
public:
    explicit SceneManager(DeviceRegistry& devices);

    PaperScene& paper() { return paper_; }
    GuillotineScene& guillotine() { return guillotine_; }
    TableScene& table() { return table_; }
    CheckScene& check() { return check_; }
    EmergencyScene& emergency() { return emergency_; }

    SceneTaskStatus status(SceneTaskId taskId) const;

    SceneTaskId paperMove(float mm, uint16_t speedMmS, uint32_t timeoutMs = 0);
    SceneTaskId guillotineCut(uint16_t speedMmS = 0, uint32_t timeoutMs = 0);
    SceneTaskId tableUp(uint32_t timeoutMs = 0);
    SceneTaskId tableDown(uint32_t timeoutMs = 0);

private:
    PaperScene paper_;
    GuillotineScene guillotine_;
    TableScene table_;
    CheckScene check_;
    EmergencyScene emergency_;
};
