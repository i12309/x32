#include "Service/CAN.h"

#include <cmath>

#include "Core.h"
#include "Service/Log.h"

namespace {

constexpr uint32_t kAckTimeoutMs = 150;
constexpr uint32_t kMoveTimeoutMs = 30000;
constexpr uint32_t kDetectTimeoutMs = 45000;
constexpr uint32_t kCutTimeoutMs = 10000;
constexpr uint32_t kCheckTimeoutMs = 300;

// Таймаут короткого ACK берется из config.CAN.timeouts_ms.ack,
// а constexpr остается только безопасным дефолтом на случай отсутствия поля.
uint32_t ackTimeoutMs() {
    const uint32_t value = Core::config.can.ackTimeoutMs;
    return value > 0 ? value : kAckTimeoutMs;
}

// Таймаут проверки статуса ноды берется из config.CAN.timeouts_ms.check.
uint32_t checkTimeoutMs() {
    const uint32_t value = Core::config.can.checkTimeoutMs;
    return value > 0 ? value : kCheckTimeoutMs;
}

// Переводит человекочитаемое значение bitrate из JSON (500) в enum Can32.
bool bitrateFromKbps(int kbps, canfw::CanBitrate& out) {
    switch (kbps) {
        case 125:
            out = canfw::CanBitrate::K125;
            return true;
        case 250:
            out = canfw::CanBitrate::K250;
            return true;
        case 500:
            out = canfw::CanBitrate::K500;
            return true;
        case 1000:
            out = canfw::CanBitrate::K1000;
            return true;
        default:
            return false;
    }
}

} // namespace

CAN& CAN::instance() {
    static CAN machine;
    return machine;
}

CAN::CAN() = default;

bool CAN::begin() {
    if (ready_) return true;

    canfw::CanBitrate bitrate = canfw::CanBitrate::K500;
    if (!bitrateFromKbps(Core::config.can.bitrate, bitrate)) {
        return setError(String("Unsupported CAN bitrate: ") + String(Core::config.can.bitrate));
    }

    bus_.reset(new canfw::Esp32TwaiBus(Core::config.can.tx, Core::config.can.rx, bitrate));
    network_.reset(new canfw::CanNetwork(*bus_));

    ready_ = bus_->begin();
    if (!ready_) {
        return setError("CAN bus begin failed");
    }

    Log::D("[CAN] started: TX=%d RX=%d bitrate=%d",
           Core::config.can.tx,
           Core::config.can.rx,
           Core::config.can.bitrate);
    return true;
}

bool CAN::checkAll() {
    if (!begin()) return false;

    bool ok = true;
    if (Core::config.nodes.empty()) {
        return setError("CAN nodes list is empty");
    }

    for (const auto& node : Core::config.nodes) {
        ok = checkScenarioNode(node.canID, node.name.c_str()) && ok;
    }
    return ok;
}

bool CAN::stopAll() {
    if (!ready_) return true;

    bool ok = true;
    for (const auto& node : Core::config.nodes) {
        ok = network_->device<Scenario::Client>(node.canID).cancel(ackTimeoutMs()) && ok;
    }
    return ok;
}

bool CAN::tableUp(Catalog::SPEED speed) {
    uint16_t address = 0;
    if (!nodeAddress("TABLE", address)) return false;
    return runScenario(address,
                       Scenario::Id::TableUp,
                       0,
                       speedOption(speed),
                       kMoveTimeoutMs);
}

bool CAN::tableDown(Catalog::SPEED speed) {
    uint16_t address = 0;
    if (!nodeAddress("TABLE", address)) return false;
    return runScenario(address,
                       Scenario::Id::TableDown,
                       0,
                       speedOption(speed),
                       kMoveTimeoutMs);
}

bool CAN::guillotineHome(Catalog::SPEED speed) {
    uint16_t address = 0;
    if (!nodeAddress("GUILLOTINE", address)) return false;
    return runScenario(address,
                       Scenario::Id::GuillotineHome,
                       0,
                       speedOption(speed),
                       kMoveTimeoutMs);
}

bool CAN::guillotineCut() {
    uint16_t address = 0;
    if (!nodeAddress("GUILLOTINE", address)) return false;
    return runScenario(address,
                       Scenario::Id::GuillotineCut,
                       0,
                       speedOption(Catalog::SPEED::Normal),
                       kCutTimeoutMs);
}

bool CAN::paperZeroPosition() {
    uint16_t address = 0;
    if (!nodeAddress("PAPER", address)) return false;
    return runScenario(address,
                       Scenario::Id::PaperZeroPosition,
                       0,
                       0,
                       ackTimeoutMs());
}

bool CAN::paperMoveSteps(int32_t steps, bool blocking) {
    uint16_t address = 0;
    if (!nodeAddress("PAPER", address)) return false;

    if (blocking) {
        return runScenario(address,
                           Scenario::Id::PaperMoveSteps,
                           steps,
                           speedOption(Catalog::SPEED::Normal),
                           kMoveTimeoutMs);
    }

    return startScenario(address,
                         Scenario::Id::PaperMoveSteps,
                         steps,
                         speedOption(Catalog::SPEED::Normal),
                         0,
                         true);
}

bool CAN::paperMoveMm(float mm, float ratioMm, bool blocking) {
    bool ok = false;
    const int32_t steps = mmToSteps(mm, ratioMm, ok);
    return ok && paperMoveSteps(steps, blocking);
}

bool CAN::paperMoveWithThrowMm(float mm, float ratioMm) {
    bool ok = false;
    const int32_t steps = mmToSteps(mm, ratioMm, ok);
    if (!ok) return false;

    uint16_t groupAddress = 0;
    uint16_t paperAddress = 0;
    uint16_t throwAddress = 0;
    if (!groupFeedThrowAddress(groupAddress)) return false;
    if (!nodeAddress("PAPER", paperAddress)) return false;
    if (!nodeAddress("THROW", throwAddress)) return false;

    // Групповая команда: PAPER и THROW должны слушать общий group из node config,
    // чтобы стартовать от одного CAN-кадра без рассинхронизации.
    if (!startScenario(groupAddress,
                       Scenario::Id::PaperThrowGroup,
                       steps,
                       speedOption(Catalog::SPEED::Normal),
                       0,
                       false)) {
        return false;
    }

    ScenarioResult paperResult;
    ScenarioResult throwResult;
    const bool paperDone = waitScenario(paperAddress, kMoveTimeoutMs, paperResult);
    const bool throwDone = waitScenario(throwAddress, kMoveTimeoutMs, throwResult);
    return paperDone && throwDone;
}

bool CAN::detectPaper(int32_t maxSteps, ScenarioResult& result) {
    uint16_t address = 0;
    if (!nodeAddress("PAPER", address)) return false;
    return runScenario(address,
                       Scenario::Id::DetectPaper,
                       maxSteps,
                       static_cast<uint16_t>(Catalog::OpticalSensor::EDGE),
                       kDetectTimeoutMs,
                       &result);
}

bool CAN::detectMark(int32_t maxSteps, ScenarioResult& result) {
    uint16_t address = 0;
    if (!nodeAddress("PAPER", address)) return false;
    return runScenario(address,
                       Scenario::Id::DetectMark,
                       maxSteps,
                       static_cast<uint16_t>(Catalog::OpticalSensor::MARK),
                       kDetectTimeoutMs,
                       &result);
}

bool CAN::throwRun(int32_t steps) {
    uint16_t address = 0;
    if (!nodeAddress("THROW", address)) return false;
    return runScenario(address,
                       Scenario::Id::ThrowRun,
                       steps,
                       speedOption(Catalog::SPEED::Normal),
                       kMoveTimeoutMs);
}

uint16_t CAN::speedOption(Catalog::SPEED speed) const {
    return static_cast<uint16_t>(speed);
}

bool CAN::runScenario(uint16_t address,
                             Scenario::Id scenario,
                             int32_t value,
                             uint16_t option,
                             uint32_t timeoutMs,
                             ScenarioResult* out) {
    if (!startScenario(address, scenario, value, option, 0, true)) {
        return false;
    }

    ScenarioResult result;
    if (!waitScenario(address, timeoutMs, result)) {
        if (out) *out = result;
        return false;
    }

    if (out) *out = result;
    return true;
}

bool CAN::startScenario(uint16_t address,
                               Scenario::Id scenario,
                               int32_t value,
                               uint16_t option,
                               uint8_t flags,
                               bool waitAck) {
    if (!begin()) return false;

    Scenario::Client client = network_->device<Scenario::Client>(address);
    const bool ok = waitAck
        ? client.start(scenario, value, option, flags, ackTimeoutMs())
        : client.startNoAck(scenario, value, option, flags, ackTimeoutMs());

    if (!ok) {
        return setError(String("Scenario start failed: id=") + String(static_cast<int>(scenario)) +
                        " can=0x" + String(address, HEX));
    }
    return true;
}

bool CAN::waitScenario(uint16_t address, uint32_t timeoutMs, ScenarioResult& out) {
    if (!begin()) return false;

    Scenario::Client client = network_->device<Scenario::Client>(address);
    Scenario::Result result;
    const bool ok = client.waitUntilDone(result, timeoutMs);
    out.ok = ok;
    out.status = result.status;
    out.remoteError = result.error;
    out.value = result.value;

    if (!ok) {
        return setError(String("Scenario failed: can=0x") + String(address, HEX) +
                        " status=" + String(static_cast<int>(result.status)) +
                        " remoteError=" + String(result.error));
    }
    return true;
}

bool CAN::checkScenarioNode(uint16_t address, const char* name) {
    Scenario::Client client = network_->device<Scenario::Client>(address);
    Scenario::Result result;
    if (!client.readStatus(result, checkTimeoutMs())) {
        return setError(String("CAN node not responding: ") + name +
                        " can=0x" + String(address, HEX));
    }

    if (result.status == Scenario::Status::Error ||
        result.status == Scenario::Status::Timeout) {
        return setError(String("CAN node reports error: ") + name +
                        " status=" + String(static_cast<int>(result.status)) +
                        " remoteError=" + String(result.error));
    }

    Log::D("[CAN] node OK: %s can=0x%s status=%d",
           name,
           String(address, HEX).c_str(),
           static_cast<int>(result.status));
    return true;
}

bool CAN::nodeAddress(const char* nodeName, uint16_t& out) {
    if (nodeName == nullptr || nodeName[0] == '\0') {
        return setError("Empty CAN node name");
    }

    const auto* node = Core::config.findNode(String(nodeName));
    if (node == nullptr || !node->hasCanID) {
        return setError(String("CAN node is not configured: ") + nodeName);
    }

    out = node->canID;
    return true;
}

bool CAN::groupFeedThrowAddress(uint16_t& out) {
    const auto* paper = Core::config.findNode("PAPER");
    const auto* throwNode = Core::config.findNode("THROW");
    if (paper == nullptr || !paper->hasGroupID) {
        return setError("PAPER group is not configured");
    }
    if (throwNode == nullptr || !throwNode->hasGroupID) {
        return setError("THROW group is not configured");
    }
    if (paper->groupID != throwNode->groupID) {
        return setError(String("PAPER and THROW are in different CAN groups: 0x") +
                        String(paper->groupID, HEX) + " vs 0x" +
                        String(throwNode->groupID, HEX));
    }

    out = paper->groupID;
    return true;
}

bool CAN::setError(const String& message) {
    lastError_ = message;
    Log::E("[CAN] %s", message.c_str());
    return false;
}

int32_t CAN::mmToSteps(float mm, float ratioMm, bool& ok) {
    ok = false;
    if (ratioMm <= 0.0f) {
        setError("Invalid paper ratio");
        return 0;
    }

    const float raw = mm * ratioMm;
    if (raw < -8388608.0f || raw > 8388607.0f) {
        setError("Paper move is out of Scenario int24 range");
        return 0;
    }

    ok = true;
    return static_cast<int32_t>(lroundf(raw));
}
