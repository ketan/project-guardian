#include <unity.h>

#include <TenMinuteWindCapture.h>

void setUp() {}
void tearDown() {}

void test_ten_minute_wind_capture_uses_a_vector_average() {
    TenMinuteWindCapture capture;
    for (size_t index = 0; index < 120; ++index) {
        capture.add(1.0F, index % 2 == 0 ? 359.0F : 1.0F);
    }

    WindAggregate aggregate;
    TEST_ASSERT_TRUE(capture.aggregate(aggregate));
    TEST_ASSERT_FLOAT_WITHIN(0.01F, 1.0F, aggregate.sustainedSpeedMetersPerSecond);
    TEST_ASSERT_TRUE(aggregate.sustainedDirectionDegrees < 1.0F ||
                     aggregate.sustainedDirectionDegrees > 359.0F);
}

void test_ten_minute_wind_capture_uses_a_three_second_gust() {
    TenMinuteWindCapture capture;
    for (size_t index = 0; index < 120; ++index) {
        capture.add(1.0F, 0.0F);
    }
    capture.add(3.0F, 0.0F);
    capture.add(3.0F, 0.0F);
    capture.add(3.0F, 0.0F);

    WindAggregate aggregate;
    TEST_ASSERT_TRUE(capture.aggregate(aggregate));
    TEST_ASSERT_FLOAT_WITHIN(0.01F, 3.0F, aggregate.gustSpeedMetersPerSecond);
}

void test_ten_minute_wind_capture_waits_for_a_two_minute_window() {
    TenMinuteWindCapture capture;
    for (size_t index = 0; index < 119; ++index) {
        capture.add(1.0F, 0.0F);
    }

    WindAggregate aggregate;
    TEST_ASSERT_FALSE(capture.aggregate(aggregate));
}

int main(int, char **) {
    UNITY_BEGIN();
    RUN_TEST(test_ten_minute_wind_capture_uses_a_vector_average);
    RUN_TEST(test_ten_minute_wind_capture_uses_a_three_second_gust);
    RUN_TEST(test_ten_minute_wind_capture_waits_for_a_two_minute_window);
    return UNITY_END();
}
