#pragma once

#include "ConfigModelSupport.h"
#include "JsonConfig.h"

class WindyPublisherConfig : public JsonConfig {
public:
    String type = "windy";
    bool enabled = false;
    int publishIntervalSeconds = 60;
    bool hasIncludeHistoryWindowMinutes = false;
    int includeHistoryWindowMinutes = 0;
    String stationId;
    String stationPassword;
    bool stationPasswordConfigured = false;

    void toJSON(JsonObject json) const {
        json["type"] = type;
        json["enabled"] = enabled;
        json["publishIntervalSeconds"] = publishIntervalSeconds;
        if (hasIncludeHistoryWindowMinutes) {
            json["includeHistoryWindowMinutes"] = includeHistoryWindowMinutes;
        }
        json["stationId"] = stationId;
        json["stationPassword"] = stationPassword;
        json["stationPasswordConfigured"] = stationPasswordConfigured;
    }

    bool fromJSON(JsonObject json) {
        if (!json["type"].is<String>() || !json["enabled"].is<bool>() ||
            !json["publishIntervalSeconds"].is<int>() || !json["stationId"].is<String>()) {
            return false;
        }
        const String parsedType = json["type"].as<String>();
        if (!isOneOf(parsedType, {"windy"})) {
            return false;
        }
        type = parsedType;
        enabled = json["enabled"].as<bool>();
        const int parsedPublishIntervalSeconds = json["publishIntervalSeconds"].as<int>();
        if (!isIntInRange(parsedPublishIntervalSeconds, 30, 300)) {
            return false;
        }
        publishIntervalSeconds = parsedPublishIntervalSeconds;
        stationId = json["stationId"].as<String>();
        stationPassword = json["stationPassword"] | "";
        stationPasswordConfigured = !stationPassword.isEmpty() || stationPasswordConfigured;
        hasIncludeHistoryWindowMinutes = json["includeHistoryWindowMinutes"].is<int>();
        if (hasIncludeHistoryWindowMinutes) {
            const int parsedIncludeHistoryWindowMinutes = json["includeHistoryWindowMinutes"].as<int>();
            if (!isIntInRange(parsedIncludeHistoryWindowMinutes, 1, 1440)) {
                return false;
            }
            includeHistoryWindowMinutes = parsedIncludeHistoryWindowMinutes;
        } else if (!json["includeHistoryWindowMinutes"].isNull()) {
            return false;
        }
        return true;
    }

    void toHttpResponseJSON(JsonObject json) const {
        json["type"] = type;
        json["enabled"] = enabled;
        json["publishIntervalSeconds"] = publishIntervalSeconds;
        if (hasIncludeHistoryWindowMinutes) {
            json["includeHistoryWindowMinutes"] = includeHistoryWindowMinutes;
        }
        json["stationId"] = stationId;
        json["stationPasswordConfigured"] = stationPasswordConfigured;
    }

    bool fromHttpRequestJSON(JsonObject json) {
        if (!json["type"].is<String>() || !json["enabled"].is<bool>() ||
            !json["publishIntervalSeconds"].is<int>() || !json["stationId"].is<String>()) {
            return false;
        }
        const String parsedType = json["type"].as<String>();
        if (!isOneOf(parsedType, {"windy"})) {
            return false;
        }
        type = parsedType;
        enabled = json["enabled"].as<bool>();
        const int parsedPublishIntervalSeconds = json["publishIntervalSeconds"].as<int>();
        if (!isIntInRange(parsedPublishIntervalSeconds, 30, 300)) {
            return false;
        }
        publishIntervalSeconds = parsedPublishIntervalSeconds;
        stationId = json["stationId"].as<String>();
        if (json["stationPassword"].is<String>()) {
            stationPassword = json["stationPassword"].as<String>();
            stationPasswordConfigured = !stationPassword.isEmpty();
        } else if (!json["stationPassword"].isNull()) {
            return false;
        }
        hasIncludeHistoryWindowMinutes = json["includeHistoryWindowMinutes"].is<int>();
        if (hasIncludeHistoryWindowMinutes) {
            const int parsedIncludeHistoryWindowMinutes = json["includeHistoryWindowMinutes"].as<int>();
            if (!isIntInRange(parsedIncludeHistoryWindowMinutes, 1, 1440)) {
                return false;
            }
            includeHistoryWindowMinutes = parsedIncludeHistoryWindowMinutes;
        } else if (!json["includeHistoryWindowMinutes"].isNull()) {
            return false;
        }
        return true;
    }
};
