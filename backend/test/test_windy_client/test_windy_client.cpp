#include <Arduino.h>
#include <unity.h>
#include <HTTPClient.h>
#include <WiFi.h>

#include "../../src/ConfigModels.cpp"
#include "WindyClient.h"

void setUp() {}
void tearDown() {}

WindyPublisherConfig configuredWindy() {
    WindyPublisherConfig config;
    config.stationId = "station 1";
    config.stationPassword = "secret";
    return config;
}

WeatherSample sample() {
    WeatherSample value;
    value.recordedAt = "2026-08-29T12:34:56Z";
    value.temperatureC = 24.2;
    value.humidityPct = 47.5;
    value.pressureHpa = 1008.4;
    value.windSpeedMps = 5.3;
    value.windGustMps = 7.1;
    value.windDirectionDeg = 245;
    value.rainfallMm = 2.5;
    value.illuminanceLux = 18450;
    return value;
}

void test_windy_request_uses_metric_v2_parameters() {
    const String path = WindyClient::requestPath(configuredWindy(), sample());

    String expected = "/api/v2/observation/update?id=station%201&time=2026-08-29T12%3A34%3A56Z&wind=5.3&gust=7.1&winddir=245&humidity=47.5&mbar=1008.4&precip=2.5&temp=24.2&softwaretype=project-guardian%2F";
    expected += VERSION;
    expected += "%2B";
    expected += GIT_SHA;
    TEST_ASSERT_EQUAL_STRING(expected.c_str(), path.c_str());
}

void test_windy_publish_uses_http_client_with_bearer_password() {
    HTTPClient::setNextStatusCode(204);
    WiFiClient transport = WiFiClient::connectedClient();
    WindyClient client;

    TEST_ASSERT_TRUE(client.publish(transport, configuredWindy(), sample()));
    TEST_ASSERT_EQUAL(204, client.lastStatusCode());
    TEST_ASSERT_EQUAL_STRING("Bearer secret", HTTPClient::lastAuthorization().c_str());
    String expected = "https://stations.windy.com/api/v2/observation/update?id=station%201&time=2026-08-29T12%3A34%3A56Z&wind=5.3&gust=7.1&winddir=245&humidity=47.5&mbar=1008.4&precip=2.5&temp=24.2&softwaretype=project-guardian%2F";
    expected += VERSION;
    expected += "%2B";
    expected += GIT_SHA;
    TEST_ASSERT_EQUAL_STRING(expected.c_str(), HTTPClient::lastRequestedUrl().c_str());
}

int main(int, char **) {
    UNITY_BEGIN();
    RUN_TEST(test_windy_request_uses_metric_v2_parameters);
    RUN_TEST(test_windy_publish_uses_http_client_with_bearer_password);
    return UNITY_END();
}
