#pragma once

#include <Arduino.h>

namespace Screen {

class Page;

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

class Panel {
public:
    bool init();
    void process();

    void show(PageId page);
    void show(Page& page);
    void back();

    void setLoadStatus(const String& text);
    void setLoadProgressColor(uint8_t index, uint32_t color);
    void showInfo(const String& text1,
                  const String& text2,
                  const String& text3,
                  const String& okText,
                  const String& cancelText,
                  bool cancelVisible);
    void showError(const String& title, const String& message);

    PageId activePage() const { return activePage_; }
    Page* activePageObject() { return activePageObject_; }
    const Page* activePageObject() const { return activePageObject_; }

private:
    PageId activePage_ = PageId::None;
    Page* activePageObject_ = nullptr;
    Page* previousPageObject_ = nullptr;
};

} // namespace Screen
