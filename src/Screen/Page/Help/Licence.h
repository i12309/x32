#pragma once

namespace Screen {

class LicencePage {
public:
    static LicencePage& instance();
    void show();

private:
    LicencePage() = default;

    void startLicenceRequest();
};

}  // namespace Screen
