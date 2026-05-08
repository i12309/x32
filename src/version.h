#pragma once

#include <Arduino.h>

#ifndef PRODUCT_VERSION
#define PRODUCT_VERSION 1
#endif

#ifndef APP_VERSION
#define APP_VERSION "1"
#endif

#ifndef APP_BUILD_NUMBER
#define APP_BUILD_NUMBER 0
#endif

namespace Version {

inline String getVersion(int espVersion = atoi(APP_VERSION),
                                int buildNumber = APP_BUILD_NUMBER,
                                int productVersion = PRODUCT_VERSION) {
    return String(productVersion) + "." +
           String(espVersion) + "." +
           String(buildNumber);
}

} // namespace Version
