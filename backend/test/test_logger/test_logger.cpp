#define LOGGER_COMPILED_LEVEL LOGGER_LEVEL_WARNING

#include <Arduino.h>
#include <unity.h>

#include "Logger.h"
#include "Tee.h"

class BufferPrint : public Print {
public:
    String value;

    size_t write(const uint8_t *buffer, size_t size) override {
        value.append(reinterpret_cast<const char *>(buffer), size);
        return size;
    }

    size_t write(uint8_t byte) override {
        return write(&byte, 1);
    }
};

class ConsoleStream : public Stream {
public:
    explicit ConsoleStream(const char *input = "") : input(input) {}

    int available() override { return static_cast<int>(input.size() - position); }
    int read() override { return available() > 0 ? input[position++] : -1; }
    int peek() override { return available() > 0 ? input[position] : -1; }
    void flush() override {}
    size_t write(uint8_t byte) override { return write(&byte, 1); }
    size_t write(const uint8_t *buffer, size_t size) override {
        output.append(reinterpret_cast<const char *>(buffer), size);
        return size;
    }

    String output;

private:
    String input;
    size_t position = 0;
};

void echoCommand(Stream &output, const char *arguments, void *context) {
    *static_cast<bool *>(context) = true;
    output.print(arguments);
    output.write(static_cast<uint8_t>('\n'));
}

void setUp() {}
void tearDown() {}

void test_logger_formats_messages_with_their_level() {
    BufferPrint output;
    Logger logger(output);

    logger.warning("Wi-Fi retry %u", 2U);

    TEST_ASSERT_EQUAL_STRING("[0ms]\033[33m[W]\033[0m Wi-Fi retry 2\n", output.value.c_str());
}

void test_logger_filters_messages_below_its_runtime_level() {
    BufferPrint output;
    Logger logger(output);
    logger.setLevel(Logger::Level::Warning);

    logger.info("hidden");
    logger.error("visible");

    TEST_ASSERT_EQUAL_STRING("[0ms]\033[31m[E]\033[0m visible\n", output.value.c_str());
}

void test_compiled_out_macros_do_not_compile_their_arguments() {
    BufferPrint output;
    Logger logger(output);

    LOG_INFO(logger, missingSymbol);
    LOG_WARNING(logger, "visible");

    TEST_ASSERT_EQUAL_STRING("[0ms]\033[33m[W]\033[0m visible\n", output.value.c_str());
}

void test_logger_writes_to_every_tee_destination() {
    BufferPrint serial;
    BufferPrint telnet;
    Tee destinations;
    destinations.add(serial);
    destinations.add(telnet);
    Logger logger(destinations);

    LOG_WARNING(logger, "visible");

    TEST_ASSERT_EQUAL_STRING("[0ms]\033[33m[W]\033[0m visible\n", serial.value.c_str());
    TEST_ASSERT_EQUAL_STRING("[0ms]\033[33m[W]\033[0m visible\n", telnet.value.c_str());
}

void test_logger_changes_level_from_serial_commands() {
    BufferPrint output;
    Logger logger(output);
    ConsoleStream serial("level debug\n");
    ConsoleStream telnet;

    logger.handle(serial, telnet);

    TEST_ASSERT_EQUAL_INT(static_cast<int>(Logger::Level::Debug), static_cast<int>(logger.level()));
    TEST_ASSERT_EQUAL_STRING("level debug\nLogger level: debug\n", serial.output.c_str());
}

void test_logger_prints_help_to_telnet() {
    BufferPrint output;
    Logger logger(output);
    ConsoleStream serial;
    ConsoleStream telnet;

    logger.printHelp(telnet);
    logger.handle(serial, telnet);

    TEST_ASSERT_EQUAL_STRING("Commands: help | info | reset | level\n",
                             telnet.output.c_str());
    TEST_ASSERT_TRUE(output.value.isEmpty());
}

void test_logger_calls_registered_commands_with_their_arguments() {
    BufferPrint output;
    Logger logger(output);
    bool called = false;
    ConsoleStream serial;
    ConsoleStream telnet("echo weather station\n");

    TEST_ASSERT_TRUE(logger.addCommand("echo", echoCommand, &called));
    TEST_ASSERT_FALSE(logger.addCommand("help", echoCommand, &called));
    TEST_ASSERT_FALSE(logger.addCommand("info", echoCommand, &called));
    logger.handle(serial, telnet);
    logger.printHelp(telnet);

    TEST_ASSERT_TRUE(called);
    TEST_ASSERT_EQUAL_STRING("weather station\n"
                             "Commands: help | info | reset | level | echo\n",
                             telnet.output.c_str());
}

void test_logger_prints_built_in_info() {
    BufferPrint output;
    Logger logger(output);
    ConsoleStream serial("info\n");
    ConsoleStream telnet;

    logger.handle(serial, telnet);

    TEST_ASSERT_EQUAL_STRING("info\nProject Guardian information\nVersion: " PROJECT_VERSION " (" GIT_SHA ")\nUptime (ms): 0\nLogger level: info\n",
                             serial.output.c_str());
}

void test_logger_runs_built_in_reset() {
    BufferPrint output;
    Logger logger(output);
    ConsoleStream serial("reset\n");
    ConsoleStream telnet;

    logger.handle(serial, telnet);

    TEST_ASSERT_EQUAL_STRING("reset\nRestarting...\n", serial.output.c_str());
}

int main(int, char **) {
    UNITY_BEGIN();
    RUN_TEST(test_logger_formats_messages_with_their_level);
    RUN_TEST(test_logger_filters_messages_below_its_runtime_level);
    RUN_TEST(test_compiled_out_macros_do_not_compile_their_arguments);
    RUN_TEST(test_logger_writes_to_every_tee_destination);
    RUN_TEST(test_logger_changes_level_from_serial_commands);
    RUN_TEST(test_logger_prints_help_to_telnet);
    RUN_TEST(test_logger_calls_registered_commands_with_their_arguments);
    RUN_TEST(test_logger_prints_built_in_info);
    RUN_TEST(test_logger_runs_built_in_reset);
    return UNITY_END();
}
