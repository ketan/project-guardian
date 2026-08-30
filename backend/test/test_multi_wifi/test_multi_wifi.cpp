#define LOGGER_COMPILED_LEVEL LOGGER_LEVEL_OFF

#include <Arduino.h>
#include <Preferences.h>
#include <unity.h>

#include <Logger.h>
#include "MultiWiFi.h"

class NullPrint : public Print {
public:
    size_t write(uint8_t) override { return 1; }
    size_t write(const uint8_t *, size_t size) override { return size; }
};

NullPrint nullPrint;
Logger logger(nullPrint);

void setUp() { Preferences::clear(); }
void tearDown() {}

void test_multi_wifi_starts_portal_without_saved_networks() {
    MultiWiFi wifi;

    TEST_ASSERT_FALSE(wifi.begin("guardian-admin", "guardian123"));
    TEST_ASSERT_TRUE(wifi.isPortalActive());
}

int main(int, char **) {
    UNITY_BEGIN();
    RUN_TEST(test_multi_wifi_starts_portal_without_saved_networks);
    return UNITY_END();
}
