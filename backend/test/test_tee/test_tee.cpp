#include <Arduino.h>
#include <unity.h>

#include "Tee.h"

class BufferPrint : public Print {
public:
    String value;
    size_t limit = SIZE_MAX;

    size_t write(const uint8_t *buffer, size_t size) override {
        const size_t written = size < limit ? size : limit;
        value.append(reinterpret_cast<const char *>(buffer), written);
        return written;
    }

    size_t write(uint8_t byte) override {
        return write(&byte, 1);
    }
};

void setUp() {
}

void tearDown() {
}

void test_tee_writes_to_every_destination() {
    BufferPrint serial;
    BufferPrint file;
    Tee tee;
    tee.add(serial);
    tee.add(file);

    TEST_ASSERT_EQUAL_UINT(2, tee.print("ok"));
    TEST_ASSERT_EQUAL_STRING("ok", serial.value.c_str());
    TEST_ASSERT_EQUAL_STRING("ok", file.value.c_str());
}

void test_tee_reports_the_shortest_write() {
    BufferPrint serial;
    BufferPrint file;
    file.limit = 1;
    Tee tee;
    tee.add(serial);
    tee.add(file);

    TEST_ASSERT_EQUAL_UINT(1, tee.print("ok"));
    TEST_ASSERT_EQUAL_STRING("ok", serial.value.c_str());
    TEST_ASSERT_EQUAL_STRING("o", file.value.c_str());
}

void test_tee_has_a_fixed_destination_limit() {
    BufferPrint destination;
    Tee tee;

    for (size_t index = 0; index < Tee::maxDestinations; ++index) {
        TEST_ASSERT_TRUE(tee.add(destination));
    }
    TEST_ASSERT_FALSE(tee.add(destination));
}

int main(int, char **) {
    UNITY_BEGIN();
    RUN_TEST(test_tee_writes_to_every_destination);
    RUN_TEST(test_tee_reports_the_shortest_write);
    RUN_TEST(test_tee_has_a_fixed_destination_limit);
    return UNITY_END();
}
