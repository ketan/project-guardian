#pragma once

#include <Arduino.h>

#include <map>

class Preferences {
public:
    bool begin(const char *, bool) { return true; }
    uint8_t getUChar(const char *key, uint8_t fallback) const {
        const auto value = numbers().find(key);
        return value == numbers().end() ? fallback : value->second;
    }
    String getString(const char *key, const char *fallback) const {
        const auto value = strings().find(key);
        return value == strings().end() ? String(fallback) : value->second;
    }
    void putUChar(const char *key, uint8_t value) { numbers()[key] = value; }
    void putString(const char *key, const String &value) { strings()[key] = value; }
    static void clear() { numbers().clear(); strings().clear(); }

private:
    static std::map<std::string, uint8_t> &numbers() {
        static std::map<std::string, uint8_t> values;
        return values;
    }
    static std::map<std::string, String> &strings() {
        static std::map<std::string, String> values;
        return values;
    }
};
