#include "Scene/Base.h"

SceneTaskId BaseScene::nextSceneTaskId_ = 1;
SceneTask BaseScene::tasks_[BaseScene::MaxSceneTasks] = {};
uint8_t BaseScene::nextSlot_ = 0;

BaseScene::BaseScene(DeviceRegistry& devices) : devices_(devices) {}

DeviceTaskId BaseScene::sendToRole(Role role, DeviceCommand command, const DeviceParams& params, uint32_t timeoutMs) {
    return devices_.sendTask(role, command, params, timeoutMs);
}

DeviceTaskId BaseScene::sendToDevice(DeviceName name, DeviceCommand command, const DeviceParams& params, uint32_t timeoutMs) {
    return devices_.sendTask(name, command, params, timeoutMs);
}

SceneTaskId BaseScene::trackOne(DeviceTaskId taskId) {
    return trackMany(&taskId, taskId == 0 ? 0 : 1);
}

SceneTaskId BaseScene::trackMany(const DeviceTaskId* taskIds, uint8_t count) {
    SceneTask& task = tasks_[nextSlot_++ % MaxSceneTasks];
    task = SceneTask();
    task.id = nextSceneTaskId_++;
    if (task.id == 0) task.id = nextSceneTaskId_++;

    const uint8_t limit = count > SceneTask::MaxDeviceTasks ? SceneTask::MaxDeviceTasks : count;
    for (uint8_t i = 0; i < limit; ++i) {
        if (taskIds[i] == 0) continue;
        task.deviceTasks[task.count++] = taskIds[i];
    }
    return task.id;
}

SceneTaskStatus BaseScene::status(SceneTaskId taskId) const {
    const SceneTask* task = findTask(taskId);
    if (task == nullptr || task->count == 0) return SceneTaskStatus::Unknown;

    SceneTaskStatus result = SceneTaskStatus::Done;
    for (uint8_t i = 0; i < task->count; ++i) {
        result = mergeStatus(result, mapDeviceStatus(devices_.taskStatus(task->deviceTasks[i])));
    }
    return result;
}

DeviceStatus BaseScene::roleStatus(Role role) const {
    return devices_.status(role);
}

bool BaseScene::roleReady(Role role) const {
    DeviceStatus status = roleStatus(role);
    return status.online && status.ready && !status.busy && !status.error && !status.incompatible;
}

String BaseScene::deviceError(Role role, const char* action) const {
    DeviceStatus status = roleStatus(role);
    String message = "[Scene] ";
    message += action ? action : "task";
    message += " failed for role=";
    message += roleName(role);
    if (status.errorText.length() > 0) {
        message += ": ";
        message += status.errorText;
    }
    return message;
}

const SceneTask* BaseScene::findTask(SceneTaskId taskId) const {
    if (taskId == 0) return nullptr;
    for (uint8_t i = 0; i < MaxSceneTasks; ++i) {
        if (tasks_[i].id == taskId) return &tasks_[i];
    }
    return nullptr;
}

SceneTaskStatus BaseScene::mapDeviceStatus(DeviceTaskStatus status) {
    switch (status) {
        case DeviceTaskStatus::Queued:
        case DeviceTaskStatus::Sent:
        case DeviceTaskStatus::Accepted:
            return SceneTaskStatus::Queued;
        case DeviceTaskStatus::Running:
            return SceneTaskStatus::Running;
        case DeviceTaskStatus::Done:
            return SceneTaskStatus::Done;
        case DeviceTaskStatus::Failed:
            return SceneTaskStatus::Failed;
        case DeviceTaskStatus::Timeout:
            return SceneTaskStatus::Timeout;
        case DeviceTaskStatus::Rejected:
            return SceneTaskStatus::Rejected;
        case DeviceTaskStatus::Unknown:
        default:
            return SceneTaskStatus::Unknown;
    }
}

SceneTaskStatus BaseScene::mergeStatus(SceneTaskStatus current, SceneTaskStatus next) {
    if (next == SceneTaskStatus::Failed || current == SceneTaskStatus::Failed) return SceneTaskStatus::Failed;
    if (next == SceneTaskStatus::Timeout || current == SceneTaskStatus::Timeout) return SceneTaskStatus::Timeout;
    if (next == SceneTaskStatus::Rejected || current == SceneTaskStatus::Rejected) return SceneTaskStatus::Rejected;
    if (next == SceneTaskStatus::Unknown || current == SceneTaskStatus::Unknown) return SceneTaskStatus::Unknown;
    if (next == SceneTaskStatus::Running || current == SceneTaskStatus::Running) return SceneTaskStatus::Running;
    if (next == SceneTaskStatus::Queued || current == SceneTaskStatus::Queued) return SceneTaskStatus::Queued;
    return SceneTaskStatus::Done;
}
