#include "Service/CAN/CanScenario.h"

#include <cmath>

#include "Core.h"
#include "Service/CAN/CAN.h"
#include "Service/Log.h"

namespace {

constexpr uint32_t kAckTimeoutMs = 150;
constexpr uint32_t kMoveTimeoutMs = 30000;
constexpr uint32_t kDetectTimeoutMs = 45000;
constexpr uint32_t kCutTimeoutMs = 10000;
constexpr uint32_t kCheckTimeoutMs = 300;

} // namespace

CanScenario::CanScenario(CanBus& bus, CanHelper& helper)
    : bus_(bus), helper_(helper) {}

bool CanScenario::checkAll() {
    if (!bus_.begin()) return setError(bus_.lastError());

    if (Core::config.nodes.empty()) {
        return setError("CAN nodes list is empty");
    }

    bool ok = true;
    for (const auto& node : Core::config.nodes) {
        if (node.canID == 0) {
            ok = setError(String("CAN node is not configured: ") + node.name) && ok;
            continue;
        }
        ok = checkScenarioNode(node.canID, node.name.c_str()) && ok;
    }
    return ok;
}

bool CanScenario::stopAll() {
    if (!bus_.isReady()) return true;

    bool ok = true;
    for (const auto& node : Core::config.nodes) {
        if (node.canID != 0) {
            ok = bus_.network().device<Scenario::Client>(node.canID).cancel(kAckTimeoutMs) && ok;
        }
    }
    return ok;
}

bool CanScenario::tableUp(Catalog::SPEED speed) {
    uint16_t address = 0;
    if (!nodeAddress("TABLE", address)) return false;
    return runScenario(address,
                       Scenario::Id::TableUp,
                       0,
                       speedOption(speed),
                       kMoveTimeoutMs);
}

bool CanScenario::tableDown(Catalog::SPEED speed) {
    uint16_t address = 0;
    if (!nodeAddress("TABLE", address)) return false;
    return runScenario(address,
                       Scenario::Id::TableDown,
                       0,
                       speedOption(speed),
                       kMoveTimeoutMs);
}

bool CanScenario::guillotineHome(Catalog::SPEED speed) {
    uint16_t address = 0;
    if (!nodeAddress("GUILLOTINE", address)) return false;
    return runScenario(address,
                       Scenario::Id::GuillotineHome,
                       0,
                       speedOption(speed),
                       kMoveTimeoutMs);
}

bool CanScenario::guillotineCut() {
    uint16_t address = 0;
    if (!nodeAddress("GUILLOTINE", address)) return false;
    return runScenario(address,
                       Scenario::Id::GuillotineCut,
                       0,
                       speedOption(Catalog::SPEED::Normal),
                       kCutTimeoutMs);
}

bool CanScenario::paperZeroPosition() {
    uint16_t address = 0;
    if (!nodeAddress("PAPER", address)) return false;
    return runScenario(address,
                       Scenario::Id::PaperZeroPosition,
                       0,
                       0,
                       kAckTimeoutMs);
}

bool CanScenario::paperMoveSteps(int32_t steps, bool blocking) {
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

bool CanScenario::paperMoveMm(float mm, float ratioMm, bool blocking) {
    bool ok = false;
    const int32_t steps = mmToSteps(mm, ratioMm, ok);
    return ok && paperMoveSteps(steps, blocking);
}

bool CanScenario::paperMoveWithThrowMm(float mm, float ratioMm) {
    bool ok = false;
    const int32_t steps = mmToSteps(mm, ratioMm, ok);
    if (!ok) return false;

    uint16_t groupAddress = 0;
    uint16_t paperAddress = 0;
    uint16_t throwAddress = 0;
    if (!feedGroup(groupAddress)) return false;
    if (!nodeAddress("PAPER", paperAddress)) return false;
    if (!nodeAddress("THROW", throwAddress)) return false;

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

bool CanScenario::detectPaper(int32_t maxSteps, ScenarioResult& result) {
    uint16_t address = 0;
    if (!nodeAddress("PAPER", address)) return false;
    return runScenario(address,
                       Scenario::Id::DetectPaper,
                       maxSteps,
                       static_cast<uint16_t>(Catalog::OpticalSensor::EDGE),
                       kDetectTimeoutMs,
                       &result);
}

bool CanScenario::detectMark(int32_t maxSteps, ScenarioResult& result) {
    uint16_t address = 0;
    if (!nodeAddress("PAPER", address)) return false;
    return runScenario(address,
                       Scenario::Id::DetectMark,
                       maxSteps,
                       static_cast<uint16_t>(Catalog::OpticalSensor::MARK),
                       kDetectTimeoutMs,
                       &result);
}

bool CanScenario::throwRun(int32_t steps) {
    uint16_t address = 0;
    if (!nodeAddress("THROW", address)) return false;
    return runScenario(address,
                       Scenario::Id::ThrowRun,
                       steps,
                       speedOption(Catalog::SPEED::Normal),
                       kMoveTimeoutMs);
}

uint16_t CanScenario::speedOption(Catalog::SPEED speed) const {
    return static_cast<uint16_t>(speed);
}

bool CanScenario::runScenario(uint16_t address,
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

bool CanScenario::startScenario(uint16_t address,
                                       Scenario::Id scenario,
                                       int32_t value,
                                       uint16_t option,
                                       uint8_t flags,
                                       bool waitAck) {
    if (!bus_.begin()) return setError(bus_.lastError());

    Scenario::Client client = bus_.network().device<Scenario::Client>(address);
    const bool ok = waitAck
        ? client.start(scenario, value, option, flags, kAckTimeoutMs)
        : client.startNoAck(scenario, value, option, flags, kAckTimeoutMs);

    if (!ok) {
        return setError(String("Scenario start failed: id=") + String(static_cast<int>(scenario)) +
                        " can=0x" + String(address, HEX));
    }
    return true;
}

bool CanScenario::waitScenario(uint16_t address, uint32_t timeoutMs, ScenarioResult& out) {
    if (!bus_.begin()) return setError(bus_.lastError());

    Scenario::Client client = bus_.network().device<Scenario::Client>(address);
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

bool CanScenario::checkScenarioNode(uint16_t address, const char* name) {
    Scenario::Client client = bus_.network().device<Scenario::Client>(address);
    Scenario::Result result;
    if (!client.readStatus(result, kCheckTimeoutMs)) {
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

bool CanScenario::nodeAddress(const char* nodeName, uint16_t& out) {
    if (helper_.nodeAddress(nodeName, out)) return true;
    return setError(helper_.lastError());
}

bool CanScenario::feedGroup(uint16_t& out) {
    if (helper_.feedGroup(out)) return true;
    return setError(helper_.lastError());
}

bool CanScenario::setError(const String& message) {
    lastError_ = message;
    Log::E("[CAN] %s", message.c_str());
    return false;
}

int32_t CanScenario::mmToSteps(float mm, float ratioMm, bool& ok) {
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
