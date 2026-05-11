#pragma once

#include <Arduino.h>
#include <memory>
#include <vector>

#include "Catalog.h"
#include "CanNetwork.h"
#include "backends/esp32/Esp32TwaiBus.h"
#include "protocols/mgmt/Mgmt.h"
#include "protocols/scenario/Scenario.h"

// Результат выполнения удаленного сценария на ноде.
// Хранит и транспортный статус ожидания, и код ошибки, присланный самой нодой.
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
    String macToString(const Mgmt::MacAddress& mac) const;
    bool allBootNodesAssigned(const std::vector<bool>& assigned) const;
    String missingBootNodes(const std::vector<bool>& assigned) const;
    // Возвращает CAN ID ноды по имени из списка config.nodes.
    bool nodeAddress(const char* nodeName, uint16_t& out);
    // Возвращает group ID для синхронного старта PAPER+THROW.
    // Сейчас обе ноды должны иметь одинаковый node.group.
    bool groupFeedThrowAddress(uint16_t& out);
    bool setError(const String& message);
    int32_t mmToSteps(float mm, float ratioMm, bool& ok);

    // TWAI bus создается после загрузки config.json, но физические параметры шины
    // остаются константами кода и не читаются из пользовательского конфига.
    std::unique_ptr<canfw::Esp32TwaiBus> bus_;
    std::unique_ptr<canfw::CanNetwork> network_;
    bool ready_ = false;
    String lastError_;
};
