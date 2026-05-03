#pragma once

#include <Arduino.h>

class Panel {
public:
    static void init();
    static void process();
    static bool isInitialized();
};
