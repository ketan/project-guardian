#pragma once

#include "ConfigModelSupport.h"
#include "JsonConfig.h"

class SamplingConfig : public JsonConfig {
public:
    int intervalSeconds = 10;
    int adminWindowMinutes = 10;
    bool deepSleepEnabled = true;
    int wakeDurationSeconds = 5;
    int historyAggregationMinutes = 30;

    void toJSON(JsonObject json) const {
        json["intervalSeconds"] = intervalSeconds;
        json["adminWindowMinutes"] = adminWindowMinutes;
        json["deepSleepEnabled"] = deepSleepEnabled;
        json["wakeDurationSeconds"] = wakeDurationSeconds;
        json["historyAggregationMinutes"] = historyAggregationMinutes;
    }

    bool fromJSON(JsonObject json) {
        if (!json["intervalSeconds"].is<int>() || !json["adminWindowMinutes"].is<int>() ||
            !json["deepSleepEnabled"].is<bool>()) {
            return false;
        }

        const int parsedIntervalSeconds = json["intervalSeconds"].as<int>();
        const int parsedAdminWindowMinutes = json["adminWindowMinutes"].as<int>();
        if (!isIntInRange(parsedIntervalSeconds, 2, 20) || !isIntInRange(parsedAdminWindowMinutes, 1, 60)) {
            return false;
        }

        intervalSeconds = parsedIntervalSeconds;
        adminWindowMinutes = parsedAdminWindowMinutes;
        deepSleepEnabled = json["deepSleepEnabled"].as<bool>();

        if (json["wakeDurationSeconds"].is<int>()) {
            const int parsedWakeDurationSeconds = json["wakeDurationSeconds"].as<int>();
            if (!isIntInRange(parsedWakeDurationSeconds, 1, 300)) {
                return false;
            }
            wakeDurationSeconds = parsedWakeDurationSeconds;
        } else if (!json["wakeDurationSeconds"].isNull()) {
            return false;
        }

        if (json["historyAggregationMinutes"].is<int>()) {
            const int parsedHistoryAggregationMinutes = json["historyAggregationMinutes"].as<int>();
            if (!isIntInRange(parsedHistoryAggregationMinutes, 5, 1440)) {
                return false;
            }
            historyAggregationMinutes = parsedHistoryAggregationMinutes;
        } else if (!json["historyAggregationMinutes"].isNull()) {
            return false;
        }

        return true;
    }
};
