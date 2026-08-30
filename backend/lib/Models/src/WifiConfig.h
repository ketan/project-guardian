#pragma once

#include "ConfigModelSupport.h"
#include "JsonConfig.h"

class WifiConfig : public JsonConfig {
public:
    bool enabled = true;
    String ssid = "guardian-station";
    String password;
    bool passwordConfigured = true;
    std::vector<String> allowedOrigins;

    void toJSON(JsonObject json) const {
        json["enabled"] = enabled;
        json["ssid"] = ssid;
        json["password"] = password;
        json["passwordConfigured"] = passwordConfigured;
        writeStringArray(json["allowedOrigins"].to<JsonArray>(), allowedOrigins);
    }

    bool fromJSON(JsonObject json) {
        enabled = json["enabled"] | enabled;
        ssid = json["ssid"] | ssid;
        password = json["password"] | password;
        passwordConfigured = json["passwordConfigured"] | passwordConfigured;
        if (json["allowedOrigins"].is<JsonArray>()) {
            parseStringArray(json["allowedOrigins"].as<JsonArray>(), allowedOrigins);
        }
        return true;
    }

    void toHttpResponseJSON(JsonObject json) const {
        json["enabled"] = enabled;
        json["ssid"] = ssid;
        json["passwordConfigured"] = passwordConfigured;
        writeStringArray(json["allowedOrigins"].to<JsonArray>(), allowedOrigins);
    }

    bool fromHttpRequestJSON(JsonObject json) {
        if (json["enabled"].is<bool>()) {
            enabled = json["enabled"].as<bool>();
        } else if (!json["enabled"].isNull()) {
            return false;
        }

        if (json["ssid"].is<String>()) {
            ssid = json["ssid"].as<String>();
        } else if (!json["ssid"].isNull()) {
            return false;
        }

        if (json["password"].is<String>()) {
            password = json["password"].as<String>();
            passwordConfigured = !password.isEmpty();
        } else if (!json["password"].isNull()) {
            return false;
        }

        if (json["allowedOrigins"].is<JsonArray>()) {
            parseStringArray(json["allowedOrigins"].as<JsonArray>(), allowedOrigins);
        } else if (!json["allowedOrigins"].isNull()) {
            return false;
        }
        return true;
    }
};
