#pragma once

#include <Arduino.h>

#include <cstdarg>
#include <cstdio>
#include <cstring>

#define LOGGER_LEVEL_OFF 0
#define LOGGER_LEVEL_ERROR 1
#define LOGGER_LEVEL_WARNING 2
#define LOGGER_LEVEL_INFO 3
#define LOGGER_LEVEL_DEBUG 4
#define LOGGER_LEVEL_TRACE 5

#ifndef LOGGER_COMPILED_LEVEL
#define LOGGER_COMPILED_LEVEL LOGGER_LEVEL_INFO
#endif

class Logger {
public:
    using CommandCallback = void (*)(Stream &output, const char *arguments, void *context);

    enum class Level : uint8_t {
        Error = LOGGER_LEVEL_ERROR,
        Warning = LOGGER_LEVEL_WARNING,
        Info = LOGGER_LEVEL_INFO,
        Debug = LOGGER_LEVEL_DEBUG,
        Trace = LOGGER_LEVEL_TRACE,
    };

    explicit Logger(Print &output) : output(output) {}

    void setLevel(Level level) { minimumLevel = level; }
    Level level() const { return minimumLevel; }

    static const char *levelName(Level level) {
        switch (level) {
            case Level::Error: return "error";
            case Level::Warning: return "warning";
            case Level::Info: return "info";
            case Level::Debug: return "debug";
            case Level::Trace: return "trace";
        }
        return "unknown";
    }

    void error(const char *format, ...) {
        va_list arguments;
        va_start(arguments, format);
        log(Level::Error, format, arguments);
        va_end(arguments);
    }

    void warning(const char *format, ...) {
        va_list arguments;
        va_start(arguments, format);
        log(Level::Warning, format, arguments);
        va_end(arguments);
    }

    void info(const char *format, ...) {
        va_list arguments;
        va_start(arguments, format);
        log(Level::Info, format, arguments);
        va_end(arguments);
    }

    void debug(const char *format, ...) {
        va_list arguments;
        va_start(arguments, format);
        log(Level::Debug, format, arguments);
        va_end(arguments);
    }

    void trace(const char *format, ...) {
        va_list arguments;
        va_start(arguments, format);
        log(Level::Trace, format, arguments);
        va_end(arguments);
    }

    void handle(Stream &serial, Stream &telnet) {
        read(serial, serialLine, serialLength, true);
        read(telnet, telnetLine, telnetLength, false);
    }

    bool addCommand(const char *name, CommandCallback callback, void *context = nullptr) {
        if (name == nullptr || callback == nullptr || commandCount == maxCommands || isBuiltinCommand(name)) {
            return false;
        }
        for (size_t index = 0; index < commandCount; ++index) {
            if (std::strcmp(commands[index].name, name) == 0) {
                return false;
            }
        }

        commands[commandCount++] = {name, callback, context};
        return true;
    }

    void setInfoCallback(CommandCallback callback, void *context = nullptr) {
        infoCallback = callback;
        infoContext = context;
    }

    void setResetCallback(CommandCallback callback, void *context = nullptr) {
        resetCallback = callback;
        resetContext = context;
    }

    void printHelp(Stream &output) const {
        output.print("help | info | reset | level <error|warning|info|debug|trace> | log <level> <message>");
        for (size_t index = 0; index < commandCount; ++index) {
            output.print(" | ");
            output.print(commands[index].name);
        }
        output.write(static_cast<uint8_t>('\n'));
    }

private:
    static constexpr size_t bufferSize = 256;
    static constexpr size_t maxLineLength = 95;
    static constexpr size_t maxCommands = 4;

    struct Command {
        const char *name;
        CommandCallback callback;
        void *context;
    };

    void read(Stream &input, char *line, size_t &length, bool echo) {
        while (input.available() > 0) {
            const int next = input.read();
            if (next == '\r' || next == '\n') {
                if (length > 0) {
                    if (echo) {
                        input.write(static_cast<uint8_t>('\n'));
                    }
                    line[length] = '\0';
                    execute(input, line);
                    length = 0;
                }
            } else if ((next == '\b' || next == 127) && length > 0) {
                --length;
                if (echo) {
                    input.print("\b \b");
                }
            } else if (length < maxLineLength) {
                line[length++] = static_cast<char>(next);
                if (echo) {
                    input.write(static_cast<uint8_t>(next));
                }
            }
        }
    }

    void execute(Stream &output, char *line) {
        char *arguments = std::strchr(line, ' ');
        if (arguments != nullptr) {
            *arguments++ = '\0';
            while (*arguments == ' ') {
                ++arguments;
            }
        }

        if (std::strcmp(line, "help") == 0) {
            printHelp(output);
        } else if (std::strcmp(line, "info") == 0) {
            if (infoCallback == nullptr) {
                reply(output, "Info unavailable");
            } else {
                infoCallback(output, arguments == nullptr ? "" : arguments, infoContext);
            }
        } else if (std::strcmp(line, "reset") == 0) {
            if (resetCallback == nullptr) {
                reply(output, "Reset unavailable");
            } else {
                resetCallback(output, arguments == nullptr ? "" : arguments, resetContext);
            }
        } else if (std::strcmp(line, "level") == 0) {
            setConsoleLevel(output, arguments);
        } else if (std::strcmp(line, "log") == 0) {
            writeConsoleLog(output, arguments);
        } else {
            executeCustomCommand(output, line, arguments);
        }
    }

    void executeCustomCommand(Stream &output, const char *name, const char *arguments) {
        for (size_t index = 0; index < commandCount; ++index) {
            if (std::strcmp(commands[index].name, name) == 0) {
                commands[index].callback(output, arguments == nullptr ? "" : arguments, commands[index].context);
                return;
            }
        }
        reply(output, "Unknown command; use help");
    }

    static bool isBuiltinCommand(const char *name) {
        return std::strcmp(name, "help") == 0 || std::strcmp(name, "info") == 0 ||
               std::strcmp(name, "reset") == 0 ||
               std::strcmp(name, "level") == 0 || std::strcmp(name, "log") == 0;
    }

    void setConsoleLevel(Stream &output, const char *name) {
        Level requestedLevel;
        if (!parseLevel(name, requestedLevel)) {
            reply(output, "Usage: level <error|warning|info|debug|trace>");
            return;
        }

        setLevel(requestedLevel);
        output.print("Logger level: ");
        reply(output, levelName(requestedLevel));
    }

    void writeConsoleLog(Stream &output, char *arguments) {
        if (arguments == nullptr) {
            reply(output, "Usage: log <level> <message>");
            return;
        }

        char *message = std::strchr(arguments, ' ');
        if (message == nullptr) {
            reply(output, "Usage: log <level> <message>");
            return;
        }
        *message++ = '\0';

        Level requestedLevel;
        if (!parseLevel(arguments, requestedLevel) || *message == '\0') {
            reply(output, "Usage: log <level> <message>");
            return;
        }

        switch (requestedLevel) {
            case Level::Error: error("%s", message); break;
            case Level::Warning: warning("%s", message); break;
            case Level::Info: info("%s", message); break;
            case Level::Debug: debug("%s", message); break;
            case Level::Trace: trace("%s", message); break;
        }
    }

    static bool parseLevel(const char *name, Level &level) {
        if (name == nullptr) {
            return false;
        }
        if (std::strcmp(name, "error") == 0) {
            level = Level::Error;
        } else if (std::strcmp(name, "warning") == 0) {
            level = Level::Warning;
        } else if (std::strcmp(name, "info") == 0) {
            level = Level::Info;
        } else if (std::strcmp(name, "debug") == 0) {
            level = Level::Debug;
        } else if (std::strcmp(name, "trace") == 0) {
            level = Level::Trace;
        } else {
            return false;
        }
        return true;
    }

    static void reply(Stream &output, const char *message) {
        output.print(message);
        output.write(static_cast<uint8_t>('\n'));
    }

    void log(Level level, const char *format, va_list arguments) {
        if (static_cast<uint8_t>(level) > static_cast<uint8_t>(minimumLevel)) {
            return;
        }

        char buffer[bufferSize];
        const int length = vsnprintf(buffer, sizeof(buffer), format, arguments);
        if (length < 0) {
            return;
        }

        char timestamp[16];
        snprintf(timestamp, sizeof(timestamp), "[%lums]", millis());
        output.print(timestamp);
        output.print(color(level));
        output.print(prefix(level));
        output.print("\033[0m ");
        output.write(reinterpret_cast<const uint8_t *>(buffer), length < static_cast<int>(bufferSize) ? length : bufferSize - 1);
        output.write(static_cast<uint8_t>('\n'));
    }

    static const char *prefix(Level level) {
        switch (level) {
            case Level::Error: return "[E]";
            case Level::Warning: return "[W]";
            case Level::Info: return "[I]";
            case Level::Debug: return "[D]";
            case Level::Trace: return "[T]";
        }
        return "";
    }

    static const char *color(Level level) {
        switch (level) {
            case Level::Error: return "\033[31m";
            case Level::Warning: return "\033[33m";
            case Level::Info: return "\033[32m";
            case Level::Debug: return "\033[36m";
            case Level::Trace: return "\033[90m";
        }
        return "";
    }

    Print &output;
    Level minimumLevel = Level::Info;
    char serialLine[maxLineLength + 1]{};
    char telnetLine[maxLineLength + 1]{};
    size_t serialLength = 0;
    size_t telnetLength = 0;
    Command commands[maxCommands]{};
    size_t commandCount = 0;
    CommandCallback infoCallback = nullptr;
    void *infoContext = nullptr;
    CommandCallback resetCallback = nullptr;
    void *resetContext = nullptr;
};

#if LOGGER_COMPILED_LEVEL >= LOGGER_LEVEL_ERROR
#define LOG_ERROR(logger, ...) (logger).error(__VA_ARGS__)
#else
#define LOG_ERROR(...) do {} while (false)
#endif

#if LOGGER_COMPILED_LEVEL >= LOGGER_LEVEL_WARNING
#define LOG_WARNING(logger, ...) (logger).warning(__VA_ARGS__)
#else
#define LOG_WARNING(...) do {} while (false)
#endif

#if LOGGER_COMPILED_LEVEL >= LOGGER_LEVEL_INFO
#define LOG_INFO(logger, ...) (logger).info(__VA_ARGS__)
#else
#define LOG_INFO(...) do {} while (false)
#endif

#if LOGGER_COMPILED_LEVEL >= LOGGER_LEVEL_DEBUG
#define LOG_DEBUG(logger, ...) (logger).debug(__VA_ARGS__)
#else
#define LOG_DEBUG(...) do {} while (false)
#endif

#if LOGGER_COMPILED_LEVEL >= LOGGER_LEVEL_TRACE
#define LOG_TRACE(logger, ...) (logger).trace(__VA_ARGS__)
#else
#define LOG_TRACE(...) do {} while (false)
#endif
