#include <Arduino.h>
#include <ArduinoJson.h>
#include <unity.h>

#include "DeviceStatus.h"

void setUp() {}
void tearDown() {}

void test_device_status_serializes_nested_status_and_collections() {
    DeviceStatus status;
    status.device.deviceId = "test-device";
    status.connectivity.wifi.connected = false;
    SensorStatus sensor;
    sensor.id = "weather";
    sensor.kind = "SEN0658";
    status.sensors.push_back(sensor);
    PublisherStatus publisher;
    publisher.type = "mqtt";
    publisher.enabled = true;
    status.publishers.push_back(publisher);

    JsonDocument document;
    JsonObject json = document.to<JsonObject>();
    status.toJSON(json);

    TEST_ASSERT_EQUAL_STRING("test-device", json["device"]["deviceId"]);
    TEST_ASSERT_FALSE(json["connectivity"]["wifi"]["connected"]);
    TEST_ASSERT_EQUAL_STRING("SEN0658", json["sensors"][0]["kind"]);
    TEST_ASSERT_TRUE(json["publishers"][0]["enabled"]);
}

void test_optional_status_fields_are_omitted_when_empty() {
    AdminWindowStatus status;
    JsonDocument document;
    JsonObject json = document.to<JsonObject>();

    status.toJSON(json);

    TEST_ASSERT_TRUE(json["openedAt"].isNull());
    TEST_ASSERT_TRUE(json["expiresAt"].isNull());
    TEST_ASSERT_TRUE(json["requestedBy"].isNull());
}

int main(int, char **) {
    UNITY_BEGIN();
    RUN_TEST(test_device_status_serializes_nested_status_and_collections);
    RUN_TEST(test_optional_status_fields_are_omitted_when_empty);
    return UNITY_END();
}
