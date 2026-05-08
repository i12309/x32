#pragma once

#include <Arduino.h>

#include "Catalog.h"
#include "CanNetwork.h"
#include "backends/esp32/Esp32TwaiBus.h"
#include "protocols/scenario/Scenario.h"

#ifndef FRONT32_CAN_TX_PIN
#define FRONT32_CAN_TX_PIN 27
#endif

#ifndef FRONT32_CAN_RX_PIN
#define FRONT32_CAN_RX_PIN 26
#endif

#ifndef FRONT32_CAN_TABLE_ID
#define FRONT32_CAN_TABLE_ID 0x201
#endif

#ifndef FRONT32_CAN_GUILLOTINE_ID
#define FRONT32_CAN_GUILLOTINE_ID 0x202
#endif

#ifndef FRONT32_CAN_PAPER_ID
#define FRONT32_CAN_PAPER_ID 0x203
#endif

#ifndef FRONT32_CAN_THROW_ID
#define FRONT32_CAN_THROW_ID 0x204
#endif

#ifndef FRONT32_CAN_GROUP_FEED_THROW_ID
#define FRONT32_CAN_GROUP_FEED_THROW_ID 0x220
#endif

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
    bool isReady() const { return ready_; }

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
    bool setError(const String& message);
    int32_t mmToSteps(float mm, float ratioMm, bool& ok);

    canfw::Esp32TwaiBus bus_;
    canfw::CanNetwork network_;
    bool ready_ = false;
    String lastError_;
};
