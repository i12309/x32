#include "Service/CAN/CanNodeCheck.h"

#include "Core.h"
#include "Service/Log.h"

namespace {

constexpr uint32_t kSelfTestTimeoutMs = 10000;
constexpr uint32_t kReadyTimeoutMs = 10000;
constexpr uint8_t kSelfTestRetries = 3;

} // namespace

CanNodeCheck& CanNodeCheck::instance() {
    static CanNodeCheck check;
    return check;
}

bool CanNodeCheck::checkAfterBoot(CanBus& bus, bool runSelfTest) {
    if (!bus.begin()) return setError(bus.lastError());

    if (Core::config.nodes.empty()) {
        return setError("CAN node check failed: nodes list is empty");
    }

    Mgmt::Client mgmt(bus.network().bus());

    if (runSelfTest) {
        for (const auto& node : Core::config.nodes) {
            if (node.canID == 0) {
                return setError(String("CAN node check failed: invalid CAN ID for ") + node.name);
            }
            if (!selfTestNode(mgmt, node.name, node.canID)) {
                return false;
            }
        }
    }

    for (const auto& node : Core::config.nodes) {
        if (node.canID == 0) {
            return setError(String("CAN node check failed: invalid CAN ID for ") + node.name);
        }
        if (!waitReadyNode(mgmt, node.name, node.canID)) {
            return false;
        }
    }

    Log::D("[CAN] node check complete");
    return true;
}

bool CanNodeCheck::selfTestNode(Mgmt::Client& mgmt, const String& nodeName, uint16_t canID) {
    // Повторяем только временные ошибки отправки или ожидания результата.
    // Если нода вернула SelfTestResult с ошибкой, это финальный отказ self-test.
    for (uint8_t attempt = 1; attempt <= kSelfTestRetries; ++attempt) {
        if (!mgmt.sendSelfTestStart(canID)) {
            if (attempt < kSelfTestRetries) continue;
            return setError(String("CAN node check failed: ") +
                            nodeName + " command=SelfTestStart send failed");
        }

        bool ok = false;
        uint8_t errorCode = 0;
        if (!mgmt.waitForSelfTestResult(canID, ok, errorCode, kSelfTestTimeoutMs)) {
            if (attempt < kSelfTestRetries) continue;
            return setError(String("CAN node check failed: ") +
                            nodeName + " command=SelfTestStart result timeout");
        }
        if (!ok) {
            return setError(String("CAN node check failed: ") +
                            nodeName + " command=SelfTestStart errorCode=" +
                            String(errorCode));
        }

        Log::D("[CAN] self-test OK: %s can=0x%s",
               nodeName.c_str(),
               String(canID, HEX).c_str());
        return true;
    }

    return setError(String("CAN node check failed: ") + nodeName + " command=SelfTestStart");
}

bool CanNodeCheck::waitReadyNode(Mgmt::Client& mgmt, const String& nodeName, uint16_t canID) {
    uint16_t roleFlags = 0;
    uint8_t status = 0;

    // ReadyReport не запрашивается отдельной командой: нода сама отправляет его,
    // когда config применен и self-test, если он запускался, завершен успешно.
    if (!mgmt.waitForReady(canID, roleFlags, status, kReadyTimeoutMs)) {
        return setError(String("CAN node check failed: ") +
                        nodeName + " command=ReadyReport timeout/status=" +
                        String(status) + " roleFlags=0x" +
                        String(roleFlags, HEX));
    }

    Log::D("[CAN] node ready: %s can=0x%s roleFlags=0x%s status=%u",
           nodeName.c_str(),
           String(canID, HEX).c_str(),
           String(roleFlags, HEX).c_str(),
           static_cast<unsigned>(status));
    return true;
}

bool CanNodeCheck::setError(const String& message) {
    lastError_ = message;
    Log::E("[CAN] %s", message.c_str());
    return false;
}
