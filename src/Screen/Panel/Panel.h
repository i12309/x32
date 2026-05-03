#pragma once

#include <Arduino.h>

namespace Screen {

class Page;

// Panel работает с объектами страниц, а не с общим enum-списком.
// Это позволяет добавлять новые страницы динамически, не меняя центральный
// заголовок экрана и не привязывая Page к generated id EEZ/LVGL.
class Panel {
public:
    bool init();
    void process();

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

    Page* activePageObject() { return activePageObject_; }
    const Page* activePageObject() const { return activePageObject_; }

private:
    Page* activePageObject_ = nullptr;
    Page* previousPageObject_ = nullptr;
};

} // namespace Screen
