#pragma once

#include <Arduino.h>

#include "Catalog.h"
#include "Service/CAN/CanBoot.h"
#include "Service/CAN/CanBus.h"
#include "Service/CAN/CanHelper.h"
#include "Service/CAN/CanScenario.h"
#include "protocols/scenario/Scenario.h"

struct ScenarioResult {
    bool ok = false;
    Scenario::Status status = Scenario::Status::Error;
    uint8_t remoteError = 0;
    int32_t value = 0;
};

class CAN {
public:
    static CAN& instance();

    bool begin();
    bool isReady() const;
    bool bootDiscovery();

    bool checkAll();
    bool stopAll();

    bool tableUp(Catalog::SPEED speed = Catalog::SPEED::Normal);
    bool tableDown(Catalog::SPEED speed = Catalog::SPEED::Normal);

    bool guillotineHome(Catalog::SPEED speed = Catalog::SPEED::Normal);
    bool guillotineCut();

    bool paperZeroPosition();
    bool paperMoveSteps(int32_t steps, bool blocking = true);
    bool paperMoveMm(float mm, float ratioMm, bool blocking = true);
    bool paperMoveWithThrowMm(float mm, float ratioMm);
    bool detectPaper(int32_t maxSteps, ScenarioResult& result);
    bool detectMark(int32_t maxSteps, ScenarioResult& result);

    bool throwRun(int32_t steps = 0);

    String lastError() const { return lastError_; }

private:
    CAN();

    bool setError(const String& message);

    CanBus bus_;
    CanHelper helper_;
    CanBoot boot_;
    CanScenario scenario_;
    String lastError_;
};
