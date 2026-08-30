#pragma once

#include "ConfigModelSupport.h"
#include "JsonConfig.h"
#include "WifiConfig.h"
#include "CellularConfig.h"

class NetworkConfig : public JsonConfig {
public:
    String preferredTransport = "auto";
    WifiConfig wifi;
    CellularConfig cellular;

    void toJSON(JsonObject json) const {
        json["preferredTransport"] = preferredTransport;
        wifi.toJSON(json["wifi"].to<JsonObject>());
        cellular.toJSON(json["cellular"].to<JsonObject>());
    }

    bool fromJSON(JsonObject json) {
        if (!json["preferredTransport"].is<String>() || !json["wifi"].is<JsonObject>() ||
            !json["cellular"].is<JsonObject>()) {
            return false;
        }
        const String parsedPreferredTransport = json["preferredTransport"].as<String>();
        if (!isOneOf(parsedPreferredTransport, {"wifi", "cellular", "auto"})) {
            return false;
        }
        preferredTransport = parsedPreferredTransport;
        return wifi.fromJSON(json["wifi"].as<JsonObject>()) &&
               cellular.fromJSON(json["cellular"].as<JsonObject>());
    }

    void toHttpResponseJSON(JsonObject json) const {
        json["preferredTransport"] = preferredTransport;
        wifi.toHttpResponseJSON(json["wifi"].to<JsonObject>());
        cellular.toHttpResponseJSON(json["cellular"].to<JsonObject>());
    }

    bool fromHttpRequestJSON(JsonObject json) {
        if (!json["preferredTransport"].is<String>() || !json["wifi"].is<JsonObject>() ||
            !json["cellular"].is<JsonObject>()) {
            return false;
        }
        const String parsedPreferredTransport = json["preferredTransport"].as<String>();
        if (!isOneOf(parsedPreferredTransport, {"wifi", "cellular", "auto"})) {
            return false;
        }
        preferredTransport = parsedPreferredTransport;
        return wifi.fromHttpRequestJSON(json["wifi"].as<JsonObject>()) &&
               cellular.fromHttpRequestJSON(json["cellular"].as<JsonObject>());
    }
};
