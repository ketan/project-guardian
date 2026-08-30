#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>

namespace {
    bool isIntInRange(int value, int minimum, int maximum) {
        return value >= minimum && value <= maximum;
    }

    bool isDoubleInRange(double value, double minimum, double maximum) {
        return value >= minimum && value <= maximum;
    }

    bool isOneOf(const String &value, std::initializer_list<const char *> allowed) {
        for (const char *candidate: allowed) {
            if (value == candidate) {
                return true;
            }
        }
        return false;
    }

    void writeStringArray(JsonArray array, const std::vector<String> &values) {
        for (const String &value: values) {
            array.add(value);
        }
    }

    void parseStringArray(JsonArray array, std::vector<String> &values) {
        values.clear();
        for (JsonVariant value: array) {
            values.push_back(value.as<String>());
        }
    }
} // namespace
