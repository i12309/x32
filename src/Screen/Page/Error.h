#pragma once

#include "Screen/Page/Page.h"

namespace Screen {

class Error : public Page {
public:
    static Error& instance();

protected:
    void onShow() override;

private:
    Error() : Page(SCREEN_ID_INFO) {}

    void renderError();
};

}  // namespace Screen
