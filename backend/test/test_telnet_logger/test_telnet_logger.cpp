#include <Arduino.h>
#include <unity.h>

#include "TelnetLogger.h"

void setUp() {}
void tearDown() {}

void test_telnet_logger_writes_to_connected_client() {
    WiFiClient client = WiFiClient::connectedClient();
    WiFiServer::queueClient(client);
    TelnetLogger logger;

    logger.begin();
    logger.handle();

    TEST_ASSERT_TRUE(logger.takeClientConnected());
    TEST_ASSERT_FALSE(logger.takeClientConnected());
    TEST_ASSERT_EQUAL_UINT(2, logger.print("ok"));
    TEST_ASSERT_EQUAL_STRING("ok", client.output().c_str());
}

void test_telnet_logger_discards_writes_without_a_client() {
    TelnetLogger logger;
    logger.begin();

    TEST_ASSERT_EQUAL_UINT(0, logger.print("ignored"));
}

int main(int, char **) {
    UNITY_BEGIN();
    RUN_TEST(test_telnet_logger_writes_to_connected_client);
    RUN_TEST(test_telnet_logger_discards_writes_without_a_client);
    return UNITY_END();
}
