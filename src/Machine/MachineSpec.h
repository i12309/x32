#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>

#include "Catalog.h"

// Спецификация логической CAN-топологии для типа станка.
class MachineSpec {
public:
    struct NodeInfo {
        String name;
        uint16_t canID = 0;
        uint16_t groupID = 0;
    };

    // Обязательная нода, которая должна быть объявлена в корневом config.nodes.
    struct NodeRequirement {
        String name;
        bool criticalForMotion;
    };

    // Логическая проверка обязательной группы.
    // Ожидаем, что обе ноды находятся в одном и том же ненулевом groupID,
    // полученном из секции config.groups.
    struct GroupRequirement {
        String name;
        String nodeA;
        String nodeB;
        bool criticalForMotion;
    };

    // Итог проверки.
    // errors - критичные расхождения, warnings - некритичные замечания.
    struct Report {
        std::vector<String> errors;
        std::vector<String> warnings;
        bool allowMotion = true; // В false переводим, если отсутствует критичный для движения узел.

        bool hasErrors() const { return !errors.empty(); }
        bool hasWarnings() const { return !warnings.empty(); }
    };

    // Создать спецификацию для выбранного типа станка.
    // Объект возвращается по значению и обычно используется один раз на этапе загрузки.
    static MachineSpec get(Catalog::MachineType type);

    Catalog::MachineType type() const { return type_; }

    // Построить дефолтный корневой CAN-каркас по спецификации станка.
    void fillControllerDefaults(JsonObject root) const;

    // Совместимость со старым кодом до полной миграции ConfigDefaults.
    void fillDeviceDefaults(JsonObject device) const;

    // Проверка логической топологии в корневом config:
    // обязательные ноды и обязательные групповые связи.
    Report validateControllerConfig(JsonObjectConst root, const std::vector<NodeInfo>& nodes) const;

    // Совместимость со старым кодом, где проверка вызывалась для config.device.
    // Сейчас проксируем в validateControllerConfig() для переданного объекта.
    Report validateDeviceConfig(JsonObjectConst device) const;

private:
    Catalog::MachineType type_ = Catalog::MachineType::UNKNOWN;

    std::vector<NodeRequirement> requiredNodes_;
    std::vector<GroupRequirement> requiredGroups_;

    static MachineSpec makeUnknown();
    static MachineSpec makeA();

    static bool arrayHasNode(JsonArrayConst nodes, const String& nodeName);
    static const NodeInfo* findNode(const std::vector<NodeInfo>& nodes, const String& nodeName);
};

