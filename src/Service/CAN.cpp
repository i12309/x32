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
constexpr int kCanTxPin = 17;
constexpr int kCanRxPin = 18;
constexpr int kCanBitrateKbps = 500;

uint32_t ackTimeoutMs() {
    return kAckTimeoutMs;
}

uint32_t checkTimeoutMs() {
    return kCheckTimeoutMs;
}

uint32_t bootDiscoveryWindowMs() {
    const uint32_t perNode = checkTimeoutMs() * 10;
    const uint32_t count = Core::config.nodes.empty()
        ? 1
        : static_cast<uint32_t>(Core::config.nodes.size());
    const uint32_t window = perNode * count;
    return window < 3000 ? 3000 : window;
}

} // namespace

CAN& CAN::instance() {
    static CAN machine;
    return machine;
}

CAN::CAN() = default;

bool CAN::begin() {
    if (ready_) return true;

    bus_.reset(new canfw::Esp32TwaiBus(kCanTxPin, kCanRxPin, canfw::CanBitrate::K500));
    network_.reset(new canfw::CanNetwork(*bus_));

    ready_ = bus_->begin();
    if (!ready_) {
        return setError("CAN bus begin failed");
    }

    Log::D("[CAN] started: TX=%d RX=%d bitrate=%d",
           kCanTxPin,
           kCanRxPin,
           kCanBitrateKbps);
    return true;
}

bool CAN::bootDiscovery() {
    // CAN запускается здесь с фиксированными пинами и bitrate, чтобы config.json не мог менять физику шины.
    if (!begin()) return false;

    // Discovery имеет смысл только если головной конфиг уже содержит ожидаемые ноды.
    const size_t nodeCount = Core::config.nodes.size();
    if (nodeCount == 0) {
        return setError("CAN boot discovery failed: nodes list is empty");
    }

    // Mgmt работает напрямую поверх шины: на этом этапе у нод еще нет рабочих CAN ID.
    Mgmt::Client mgmt(network_->bus());
    // assigned хранит, какие ноды уже найдены по MAC и подтвердили назначенный CAN ID.
    std::vector<bool> assigned(nodeCount, false);
    const uint32_t startedAt = millis();
    // Окно ожидания ограничивает BOOT: если нужные ноды не объявились, загрузка остановится.
    const uint32_t windowMs = bootDiscoveryWindowMs();

    Log::D("[CAN] boot discovery window: nodes=%u timeout=%u ms",
           static_cast<unsigned>(nodeCount),
           static_cast<unsigned>(windowMs));

    // Ждем BootHello до тех пор, пока все ноды не будут назначены или не закончится окно BOOT.
    while (!allBootNodesAssigned(assigned)) {
        const uint32_t elapsed = millis() - startedAt;
        if (elapsed >= windowMs) break;

        // Слушаем короткими интервалами, чтобы регулярно проверять общий таймаут окна.
        const uint32_t remaining = windowMs - elapsed;
        const uint32_t waitMs = remaining < checkTimeoutMs() ? remaining : checkTimeoutMs();

        Mgmt::BootInfo boot;
        // BootHello содержит MAC ноды и версию Mgmt-протокола.
        if (!mgmt.waitForBootHello(boot, waitMs)) {
            continue;
        }

        const String mac = macToString(boot.mac);
        // Версия протокола проверяется до назначения ID, иначе Front32 может говорить с несовместимой нодой.
        if (boot.protocolVersion != Mgmt::PROTOCOL_VERSION) {
            return setError(String("CAN boot discovery failed: incompatible Mgmt protocol from MAC ") +
                            mac + " version=" + String(boot.protocolVersion) +
                            " expected=" + String(Mgmt::PROTOCOL_VERSION));
        }

        // MAC из BootHello должен совпасть с одной из уже загруженных и проверенных node_*.json.
        String nodeName;
        uint16_t nodeCanID = 0;
        bool hasPayload = false;
        if (!Core::config.nodeBootInfoByMac(mac, nodeName, nodeCanID, hasPayload)) {
            return setError(String("CAN boot discovery failed: unknown node MAC ") + mac);
        }
        // Нода без CAN ID или payload считается невалидной: дальше ее нельзя безопасно конфигурировать.
        if (nodeCanID == 0 || !hasPayload) {
            return setError(String("CAN boot discovery failed: invalid node config for ") +
                            nodeName + " MAC " + mac);
        }

        // Находим индекс ноды, чтобы отметить ее как успешно назначенную.
        size_t nodeIndex = nodeCount;
        for (size_t i = 0; i < nodeCount; ++i) {
            if (Core::config.nodes[i].name == nodeName) {
                nodeIndex = i;
                break;
            }
        }
        if (nodeIndex == nodeCount) {
            return setError(String("CAN boot discovery failed: node config disappeared for MAC ") + mac);
        }
        // Повторный BootHello от уже назначенной ноды не ошибка: просто продолжаем ждать остальных.
        if (assigned[nodeIndex]) {
            continue;
        }

        // Назначаем ноде ее рабочий CAN ID из конфига.
        if (!mgmt.sendAssignCanID(boot.mac, nodeCanID)) {
            return setError(String("CAN boot discovery failed: AssignCanID send failed for ") +
                            nodeName + " can=0x" + String(nodeCanID, HEX));
        }
        // AssignAck подтверждает, что именно эта нода приняла назначенный CAN ID.
        if (!mgmt.waitForAssignAck(nodeCanID, ackTimeoutMs())) {
            return setError(String("CAN boot discovery failed: AssignAck timeout for ") +
                            nodeName + " can=0x" + String(nodeCanID, HEX) +
                            " MAC " + mac);
        }

        assigned[nodeIndex] = true;
        Log::D("[CAN] boot assigned: %s MAC=%s can=0x%s",
               nodeName.c_str(),
               mac.c_str(),
               String(nodeCanID, HEX).c_str());
    }

    // Если окно закончилось не для всех нод, BOOT останавливается с перечислением отсутствующих.
    if (!allBootNodesAssigned(assigned)) {
        return setError(String("CAN boot discovery timeout, missing nodes: ") +
                        missingBootNodes(assigned));
    }

    Log::D("[CAN] boot discovery complete");
    return true;
}

bool CAN::checkAll() {
    if (!begin()) return false;

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

bool CAN::stopAll() {
    if (!ready_) return true;

    bool ok = true;
    for (const auto& node : Core::config.nodes) {
        if (node.canID != 0) {
            ok = network_->device<Scenario::Client>(node.canID).cancel(ackTimeoutMs()) && ok;
        }
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

    if (!Core::config.nodeAddress(nodeName, out) || out == 0) {
        return setError(String("CAN node is not configured: ") + nodeName);
    }
    return true;
}

bool CAN::groupFeedThrowAddress(uint16_t& out) {
    uint16_t paperGroup = 0;
    uint16_t throwGroup = 0;
    if (!Core::config.nodeGroup("PAPER", paperGroup) || paperGroup == 0) {
        return setError("PAPER group is not configured");
    }
    if (!Core::config.nodeGroup("THROW", throwGroup) || throwGroup == 0) {
        return setError("THROW group is not configured");
    }
    if (paperGroup != throwGroup) {
        return setError(String("PAPER and THROW are in different CAN groups: 0x") +
                        String(paperGroup, HEX) + " vs 0x" +
                        String(throwGroup, HEX));
    }

    out = paperGroup;
    return true;
}

String CAN::macToString(const Mgmt::MacAddress& mac) const {
    char buffer[18];
    snprintf(buffer,
             sizeof(buffer),
             "%02X:%02X:%02X:%02X:%02X:%02X",
             mac.bytes[0],
             mac.bytes[1],
             mac.bytes[2],
             mac.bytes[3],
             mac.bytes[4],
             mac.bytes[5]);
    return String(buffer);
}

bool CAN::allBootNodesAssigned(const std::vector<bool>& assigned) const {
    for (bool value : assigned) {
        if (!value) return false;
    }
    return true;
}

String CAN::missingBootNodes(const std::vector<bool>& assigned) const {
    String result;
    for (size_t i = 0; i < assigned.size() && i < Core::config.nodes.size(); ++i) {
        if (assigned[i]) continue;
        if (result.length() > 0) result += ", ";
        result += Core::config.nodes[i].name;
        result += "(MAC ";
        result += Core::config.nodes[i].mac;
        result += ")";
    }
    return result;
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
