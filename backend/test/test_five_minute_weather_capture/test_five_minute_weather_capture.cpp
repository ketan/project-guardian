#include <unity.h>

#include <FiveMinuteWeatherCapture.h>

void setUp() {}
void tearDown() {}

SEN0658Readings reading(float temperature, float rainfall) {
    return {0.0F, 0, 0, 50.0F, temperature, 40.0F, 10, 20, 100.0F, 1000, rainfall};
}

void test_five_minute_weather_capture_averages_five_readings() {
    FiveMinuteWeatherCapture capture;
    for (size_t index = 0; index < 5; ++index) {
        capture.add(reading(20.0F + index, 3.0F + index * 0.1F));
    }

    FiveMinuteWeatherAggregate aggregate;
    TEST_ASSERT_TRUE(capture.aggregate(aggregate));
    TEST_ASSERT_FLOAT_WITHIN(0.001F, 22.0F, aggregate.temperatureCelsius);
}

void test_five_minute_weather_capture_requires_five_readings() {
    FiveMinuteWeatherCapture capture;
    capture.add(reading(20.0F, 3.0F));

    FiveMinuteWeatherAggregate aggregate;
    TEST_ASSERT_FALSE(capture.aggregate(aggregate));
}

void test_five_minute_weather_capture_keeps_the_latest_window() {
    FiveMinuteWeatherCapture capture;
    for (size_t index = 0; index < 6; ++index) {
        capture.add(reading(20.0F + index, 3.0F));
    }

    FiveMinuteWeatherAggregate aggregate;
    TEST_ASSERT_TRUE(capture.aggregate(aggregate));
    TEST_ASSERT_FLOAT_WITHIN(0.001F, 23.0F, aggregate.temperatureCelsius);
}

int main(int, char **) {
    UNITY_BEGIN();
    RUN_TEST(test_five_minute_weather_capture_averages_five_readings);
    RUN_TEST(test_five_minute_weather_capture_requires_five_readings);
    RUN_TEST(test_five_minute_weather_capture_keeps_the_latest_window);
    return UNITY_END();
}
