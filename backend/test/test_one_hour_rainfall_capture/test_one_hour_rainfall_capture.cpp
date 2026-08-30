#include <unity.h>

#include <OneHourRainfallCapture.h>

void setUp() {}
void tearDown() {}

void test_one_hour_rainfall_capture_waits_for_an_hour() {
    OneHourRainfallCapture capture;
    for (size_t index = 0; index < 60; ++index) capture.add(index * 0.1F);
    float rainfall;
    TEST_ASSERT_FALSE(capture.lastHour(rainfall));
}

void test_one_hour_rainfall_capture_calculates_the_rolling_delta() {
    OneHourRainfallCapture capture;
    for (size_t index = 0; index < 61; ++index) capture.add(2.0F + index * 0.1F);
    float rainfall;
    TEST_ASSERT_TRUE(capture.lastHour(rainfall));
    TEST_ASSERT_FLOAT_WITHIN(0.001F, 6.0F, rainfall);
}

int main(int, char **) {
    UNITY_BEGIN();
    RUN_TEST(test_one_hour_rainfall_capture_waits_for_an_hour);
    RUN_TEST(test_one_hour_rainfall_capture_calculates_the_rolling_delta);
    return UNITY_END();
}
