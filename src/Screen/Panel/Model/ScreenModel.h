#pragma once

#include <Arduino.h>

namespace Screen {

struct TextBinding {
    String text;

    void setText(const String& value) { text = value; }
};

struct ColorBinding {
    uint32_t color = 0;

    void setColor(uint32_t value) { color = value; }
};

struct ValueBinding {
    int32_t value = 0;
    bool visible = true;

    void setValue(int32_t nextValue) { value = nextValue; }
    void setVisible(bool nextVisible) { visible = nextVisible; }
};

} // namespace Screen
