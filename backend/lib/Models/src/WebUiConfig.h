#pragma once

#include "ConfigModelSupport.h"
#include "JsonConfig.h"

class WebUiConfig : public JsonConfig {
public:
    int tokenTtlMinutes = 15;
    std::vector<String> allowedOrigins;

    void toJSON(JsonObject json) const {
        json["tokenTtlMinutes"] = tokenTtlMinutes;
        writeStringArray(json["allowedOrigins"].to<JsonArray>(), allowedOrigins);
    }

    bool fromJSON(JsonObject json) {
        if (!json["tokenTtlMinutes"].is<int>() || !json["allowedOrigins"].is<JsonArray>()) {
            return false;
        }
        const int parsedTokenTtlMinutes = json["tokenTtlMinutes"].as<int>();
        if (!isIntInRange(parsedTokenTtlMinutes, 1, 1440)) {
            return false;
        }
        tokenTtlMinutes = parsedTokenTtlMinutes;
        parseStringArray(json["allowedOrigins"].as<JsonArray>(), allowedOrigins);
        return true;
    }
};
