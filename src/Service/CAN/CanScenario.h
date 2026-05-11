#pragma once

#include <Arduino.h>

#include "Catalog.h"
#include "Service/CAN/CanBus.h"
#include "Service/CAN/CanHelper.h"
#include "protocols/scenario/Scenario.h"

struct ScenarioResult;

class CanScenario {
public:
    CanScenario(CanBus& bus, CanHelper& helper);

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
    uint16_t speedOption(Catalog::SPEED speed) const;
    bool runScenario(uint16_t address,
                     Scenario::Id scenario,
                     int32_t value,
                     uint16_t option,
                     uint32_t timeoutMs,
                     ScenarioResult* out = nullptr);
    bool startScenario(uint16_t address,
                       Scenario::Id scenario,
                       int32_t value,
                       uint16_t option,
                       uint8_t flags,
                       bool waitAck);
    bool waitScenario(uint16_t address,
                      uint32_t timeoutMs,
                      ScenarioResult& out);
    bool checkScenarioNode(uint16_t address, const char* name);
    bool nodeAddress(const char* nodeName, uint16_t& out);
    bool feedGroup(uint16_t& out);
    bool setError(const String& message);
    int32_t mmToSteps(float mm, float ratioMm, bool& ok);

    CanBus& bus_;
    CanHelper& helper_;
    String lastError_;
};
