#include "Service/CAN/CAN.h"

CAN& CAN::instance() {
    static CAN machine;
    return machine;
}

CAN::CAN()
    : scenario_(bus_, helper_) {}

bool CAN::begin() {
    if (bus_.begin()) return true;
    return setError(bus_.lastError());
}

bool CAN::isReady() const {
    return bus_.isReady();
}

bool CAN::bootDiscovery() {
    if (boot_.discover(bus_)) return true;
    return setError(boot_.lastError());
}

bool CAN::checkAll() {
    if (scenario_.checkAll()) return true;
    return setError(scenario_.lastError());
}

bool CAN::stopAll() {
    if (scenario_.stopAll()) return true;
    return setError(scenario_.lastError());
}

bool CAN::tableUp(Catalog::SPEED speed) {
    if (scenario_.tableUp(speed)) return true;
    return setError(scenario_.lastError());
}

bool CAN::tableDown(Catalog::SPEED speed) {
    if (scenario_.tableDown(speed)) return true;
    return setError(scenario_.lastError());
}

bool CAN::guillotineHome(Catalog::SPEED speed) {
    if (scenario_.guillotineHome(speed)) return true;
    return setError(scenario_.lastError());
}

bool CAN::guillotineCut() {
    if (scenario_.guillotineCut()) return true;
    return setError(scenario_.lastError());
}

bool CAN::paperZeroPosition() {
    if (scenario_.paperZeroPosition()) return true;
    return setError(scenario_.lastError());
}

bool CAN::paperMoveSteps(int32_t steps, bool blocking) {
    if (scenario_.paperMoveSteps(steps, blocking)) return true;
    return setError(scenario_.lastError());
}

bool CAN::paperMoveMm(float mm, float ratioMm, bool blocking) {
    if (scenario_.paperMoveMm(mm, ratioMm, blocking)) return true;
    return setError(scenario_.lastError());
}

bool CAN::paperMoveWithThrowMm(float mm, float ratioMm) {
    if (scenario_.paperMoveWithThrowMm(mm, ratioMm)) return true;
    return setError(scenario_.lastError());
}

bool CAN::detectPaper(int32_t maxSteps, ScenarioResult& result) {
    if (scenario_.detectPaper(maxSteps, result)) return true;
    return setError(scenario_.lastError());
}

bool CAN::detectMark(int32_t maxSteps, ScenarioResult& result) {
    if (scenario_.detectMark(maxSteps, result)) return true;
    return setError(scenario_.lastError());
}

bool CAN::throwRun(int32_t steps) {
    if (scenario_.throwRun(steps)) return true;
    return setError(scenario_.lastError());
}

bool CAN::setError(const String& message) {
    lastError_ = message.length() > 0 ? message : String("CAN operation failed");
    return false;
}
