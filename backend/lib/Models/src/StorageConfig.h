#pragma once

#include "ConfigModelSupport.h"
#include "JsonConfig.h"

class StorageConfig : public JsonConfig {
public:
    int retentionDays = 14;
    String logFormat = "jsonl";
    String configSource = "sd_with_flash_fallback";

    void toJSON(JsonObject json) const {
        json["retentionDays"] = retentionDays;
        json["logFormat"] = logFormat;
        json["configSource"] = configSource;
    }

    bool fromJSON(JsonObject json) {
        if (!json["retentionDays"].is<int>() || !json["logFormat"].is<String>()) {
            return false;
        }
        const int parsedRetentionDays = json["retentionDays"].as<int>();
        const String parsedLogFormat = json["logFormat"].as<String>();
        if (!isIntInRange(parsedRetentionDays, 1, 30) || !isOneOf(parsedLogFormat, {"jsonl"})) {
            return false;
        }
        retentionDays = parsedRetentionDays;
        logFormat = parsedLogFormat;

        if (json["configSource"].is<String>()) {
            const String parsedConfigSource = json["configSource"].as<String>();
            if (!isOneOf(parsedConfigSource, {"sd_with_flash_fallback"})) {
                return false;
            }
            configSource = parsedConfigSource;
        } else if (!json["configSource"].isNull()) {
            return false;
        }

        return true;
    }
};
