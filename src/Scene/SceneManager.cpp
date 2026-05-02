#include "Scene/SceneManager.h"

SceneManager::SceneManager(DeviceRegistry& devices)
    : paper_(devices),
      guillotine_(devices),
      table_(devices),
      check_(devices),
      emergency_(devices) {}

SceneTaskStatus SceneManager::status(SceneTaskId taskId) const {
    return paper_.status(taskId);
}

SceneTaskId SceneManager::paperMove(float mm, uint16_t speedMmS, uint32_t timeoutMs) {
    return paper_.feed(mm, speedMmS, timeoutMs);
}

SceneTaskId SceneManager::guillotineCut(uint16_t speedMmS, uint32_t timeoutMs) {
    return guillotine_.cut(speedMmS, timeoutMs);
}

SceneTaskId SceneManager::tableUp(uint32_t timeoutMs) {
    return table_.up(timeoutMs);
}

SceneTaskId SceneManager::tableDown(uint32_t timeoutMs) {
    return table_.down(timeoutMs);
}
