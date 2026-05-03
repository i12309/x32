#include "Log.h"
#include "Core.h"
// Инициализация статических членов
const char* Log::TAG = "";
const char* Log::type = "log";
int Log::level = 0; // 0 - без дебага, 1 - с дебагом 
#if !defined(X32_TARGET_HEAD_UNIT)
MQTTc* Log::mqttClient = nullptr;
#endif
