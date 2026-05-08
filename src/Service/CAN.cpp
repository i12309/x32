#include "Service/CAN.h"

#include <cmath>

#include "Service/Log.h"

namespace {

constexpr uint32_t kAckTimeoutMs = 150;
constexpr uint32_t kMoveTimeoutMs = 30000;
constexpr uint32_t kDetectTimeoutMs = 45000;
constexpr uint32_t kCutTimeoutMs = 10000;
constexpr uint32_t kCheckTimeoutMs = 300;

} // namespace

CAN& CAN::instance() {
    static CAN machine;
    return machine;
}

CAN::CAN()
    : bus_(FRONT32_CAN_TX_PIN, FRONT32_CAN_RX_PIN, canfw::CanBitrate::K500)
    , network_(bus_) {}

bool CAN::begin() {
    if (ready_) return true;

    ready_ = bus_.begin();
    if (!ready_) {
        return setError("CAN bus begin failed");
    }

    Log::D("[CAN] started: TX=%d RX=%d", FRONT32_CAN_TX_PIN, FRONT32_CAN_RX_PIN);
    return true;
}

bool CAN::checkAll() {
    if (!begin()) return false;

    bool ok = true;
    ok = checkScenarioNode(FRONT32_CAN_TABLE_ID, "TABLE") && ok;
    ok = checkScenarioNode(FRONT32_CAN_GUILLOTINE_ID, "GUILLOTINE") && ok;
    ok = checkScenarioNode(FRONT32_CAN_PAPER_ID, "PAPER") && ok;
    ok = checkScenarioNode(FRONT32_CAN_THROW_ID, "THROW") && ok;
    return ok;
}

bool CAN::stopAll() {
    if (!ready_) return true;

    bool ok = true;
    ok = network_.device<Scenario::Client>(FRONT32_CAN_TABLE_ID).cancel(kAckTimeoutMs) && ok;
    ok = network_.device<Scenario::Client>(FRONT32_CAN_GUILLOTINE_ID).cancel(kAckTimeoutMs) && ok;
    ok = network_.device<Scenario::Client>(FRONT32_CAN_PAPER_ID).cancel(kAckTimeoutMs) && ok;
    ok = network_.device<Scenario::Client>(FRONT32_CAN_THROW_ID).cancel(kAckTimeoutMs) && ok;
    return ok;
}

bool CAN::tableUp(Catalog::SPEED speed) {
    return runScenario(FRONT32_CAN_TABLE_ID,
                       Scenario::Id::TableUp,
                       0,
                       speedOption(speed),
                       kMoveTimeoutMs);
}

bool CAN::tableDown(Catalog::SPEED speed) {
    return runScenario(FRONT32_CAN_TABLE_ID,
                       Scenario::Id::TableDown,
                       0,
                       speedOption(speed),
                       kMoveTimeoutMs);
}

bool CAN::guillotineHome(Catalog::SPEED speed) {
    return runScenario(FRONT32_CAN_GUILLOTINE_ID,
                       Scenario::Id::GuillotineHome,
                       0,
                       speedOption(speed),
                       kMoveTimeoutMs);
}

bool CAN::guillotineCut() {
    return runScenario(FRONT32_CAN_GUILLOTINE_ID,
                       Scenario::Id::GuillotineCut,
                       0,
                       speedOption(Catalog::SPEED::Normal),
                       kCutTimeoutMs);
}

bool CAN::paperZeroPosition() {
    return runScenario(FRONT32_CAN_PAPER_ID,
                       Scenario::Id::PaperZeroPosition,
                       0,
                       0,
                       kAckTimeoutMs);
}

bool CAN::paperMoveSteps(int32_t steps, bool blocking) {
    if (blocking) {
        return runScenario(FRONT32_CAN_PAPER_ID,
                           Scenario::Id::PaperMoveSteps,
                           steps,
                           speedOption(Catalog::SPEED::Normal),
                           kMoveTimeoutMs);
    }

    return startScenario(FRONT32_CAN_PAPER_ID,
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

    // Group command: PAPER and THROW nodes should both attach a Scenario::Server
    // to FRONT32_CAN_GROUP_FEED_THROW_ID and start from this single CAN frame.
    if (!startScenario(FRONT32_CAN_GROUP_FEED_THROW_ID,
                       Scenario::Id::PaperThrowGroup,
                       steps,
                       speedOption(Catalog::SPEED::Normal),
                       0,
                       false)) {
        return false;
    }

    ScenarioResult paperResult;
    ScenarioResult throwResult;
    const bool paperDone = waitScenario(FRONT32_CAN_PAPER_ID, kMoveTimeoutMs, paperResult);
    const bool throwDone = waitScenario(FRONT32_CAN_THROW_ID, kMoveTimeoutMs, throwResult);
    return paperDone && throwDone;
}

bool CAN::detectPaper(int32_t maxSteps, ScenarioResult& result) {
    return runScenario(FRONT32_CAN_PAPER_ID,
                       Scenario::Id::DetectPaper,
                       maxSteps,
                       static_cast<uint16_t>(Catalog::OpticalSensor::EDGE),
                       kDetectTimeoutMs,
                       &result);
}

bool CAN::detectMark(int32_t maxSteps, ScenarioResult& result) {
    return runScenario(FRONT32_CAN_PAPER_ID,
                       Scenario::Id::DetectMark,
                       maxSteps,
                       static_cast<uint16_t>(Catalog::OpticalSensor::MARK),
                       kDetectTimeoutMs,
                       &result);
}

bool CAN::throwRun(int32_t steps) {
    return runScenario(FRONT32_CAN_THROW_ID,
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

    Scenario::Client client = network_.device<Scenario::Client>(address);
    const bool ok = waitAck
        ? client.start(scenario, value, option, flags, kAckTimeoutMs)
        : client.startNoAck(scenario, value, option, flags, kAckTimeoutMs);

    if (!ok) {
        return setError(String("Scenario start failed: id=") + String(static_cast<int>(scenario)) +
                        " can=0x" + String(address, HEX));
    }
    return true;
}

bool CAN::waitScenario(uint16_t address, uint32_t timeoutMs, ScenarioResult& out) {
    if (!begin()) return false;

    Scenario::Client client = network_.device<Scenario::Client>(address);
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
    Scenario::Client client = network_.device<Scenario::Client>(address);
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
