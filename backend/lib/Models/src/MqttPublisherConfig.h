#pragma once

#include "ConfigModelSupport.h"
#include "JsonConfig.h"

class MqttPublisherConfig : public JsonConfig {
public:
    String type = "mqtt";
    bool enabled = false;
    int publishIntervalSeconds = 60;
    bool hasIncludeHistoryWindowMinutes = false;
    int includeHistoryWindowMinutes = 0;
    String brokerUrl = "mqtt://broker";
    String topic = "meshtastic/weather";
    String username = "guardian";
    String password;
    bool passwordConfigured = false;

    void toJSON(JsonObject json) const {
        json["type"] = type;
        json["enabled"] = enabled;
        json["publishIntervalSeconds"] = publishIntervalSeconds;
        if (hasIncludeHistoryWindowMinutes) {
            json["includeHistoryWindowMinutes"] = includeHistoryWindowMinutes;
        }
        json["brokerUrl"] = brokerUrl;
        json["topic"] = topic;
        json["username"] = username;
        json["password"] = password;
        json["passwordConfigured"] = passwordConfigured;
    }

    bool fromJSON(JsonObject json) {
        if (!json["type"].is<String>() || !json["enabled"].is<bool>() ||
            !json["publishIntervalSeconds"].is<int>() || !json["brokerUrl"].is<String>() ||
            !json["topic"].is<String>() || !json["username"].is<String>()) {
            return false;
        }
        const String parsedType = json["type"].as<String>();
        if (!isOneOf(parsedType, {"mqtt"})) {
            return false;
        }
        type = parsedType;
        enabled = json["enabled"].as<bool>();
        const int parsedPublishIntervalSeconds = json["publishIntervalSeconds"].as<int>();
        if (!isIntInRange(parsedPublishIntervalSeconds, 30, 300)) {
            return false;
        }
        publishIntervalSeconds = parsedPublishIntervalSeconds;
        brokerUrl = json["brokerUrl"].as<String>();
        topic = json["topic"].as<String>();
        username = json["username"].as<String>();
        password = json["password"] | "";
        passwordConfigured = !password.isEmpty() || passwordConfigured;
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
        json["brokerUrl"] = brokerUrl;
        json["topic"] = topic;
        json["username"] = username;
        json["passwordConfigured"] = passwordConfigured;
    }

    bool fromHttpRequestJSON(JsonObject json) {
        if (!json["type"].is<String>() || !json["enabled"].is<bool>() ||
            !json["publishIntervalSeconds"].is<int>() || !json["brokerUrl"].is<String>() ||
            !json["topic"].is<String>() || !json["username"].is<String>()) {
            return false;
        }
        const String parsedType = json["type"].as<String>();
        if (!isOneOf(parsedType, {"mqtt"})) {
            return false;
        }
        type = parsedType;
        enabled = json["enabled"].as<bool>();
        const int parsedPublishIntervalSeconds = json["publishIntervalSeconds"].as<int>();
        if (!isIntInRange(parsedPublishIntervalSeconds, 30, 300)) {
            return false;
        }
        publishIntervalSeconds = parsedPublishIntervalSeconds;
        brokerUrl = json["brokerUrl"].as<String>();
        topic = json["topic"].as<String>();
        username = json["username"].as<String>();
        if (json["password"].is<String>()) {
            password = json["password"].as<String>();
            passwordConfigured = !password.isEmpty();
        } else if (!json["password"].isNull()) {
            return false;
        }

        if (json["passwordConfigured"].is<bool>()) {
            passwordConfigured = json["passwordConfigured"].as<bool>();
        } else if (!json["passwordConfigured"].isNull()) {
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
