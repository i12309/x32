#pragma once

#include <Arduino.h>

#include "Screen/Panel/Model/MainModel.h"
#include "Screen/Panel/Model/ServiceModel.h"
#include "Screen/Panel/Model/TaskModel.h"

namespace Screen {

enum class PageId : uint8_t {
    None,
    Load,
    Main,
    Info,
    Error,
    Task,
    TaskRun,
    Service,
    Guillotine,
    Paper
};

struct PanelModel {
    MainModel main;
    TaskModel task;
    ServiceModel service;
};

class Panel {
public:
    virtual ~Panel() = default;

    virtual bool init();
    virtual void process();
    virtual void show(PageId page);

    PageId activePage() const { return activePage_; }
    PanelModel& model() { return model_; }
    const PanelModel& model() const { return model_; }

protected:
    PanelModel model_;
    PageId activePage_ = PageId::None;
};

} // namespace Screen
