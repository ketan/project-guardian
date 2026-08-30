#pragma once

#include "ConfigModelSupport.h"
#include "JsonConfig.h"

class CellularConfig : public JsonConfig {
public:
    bool enabled = false;
    String modemType = "SIM7670G";
    String apn;
    String pin;
    bool pinConfigured = false;
    bool smsEnabled = false;

    void toJSON(JsonObject json) const {
        json["enabled"] = enabled;
        json["modemType"] = modemType;
        json["apn"] = apn;
        json["pin"] = pin;
        json["pinConfigured"] = pinConfigured;
        json["smsEnabled"] = smsEnabled;
    }

    bool fromJSON(JsonObject json) {
        enabled = json["enabled"] | enabled;
        modemType = json["modemType"] | modemType;
        apn = json["apn"] | apn;
        pin = json["pin"] | pin;
        pinConfigured = json["pinConfigured"] | pinConfigured;
        smsEnabled = json["smsEnabled"] | smsEnabled;
        return true;
    }

    void toHttpResponseJSON(JsonObject json) const {
        json["enabled"] = enabled;
        json["modemType"] = modemType;
        json["apn"] = apn;
        json["pinConfigured"] = pinConfigured;
        json["smsEnabled"] = smsEnabled;
    }

    bool fromHttpRequestJSON(JsonObject json) {
        if (json["enabled"].is<bool>()) {
            enabled = json["enabled"].as<bool>();
        } else if (!json["enabled"].isNull()) {
            return false;
        }

        if (json["modemType"].is<String>()) {
            const String parsedModemType = json["modemType"].as<String>();
            if (!isOneOf(parsedModemType, {"SIM7670G"})) {
                return false;
            }
            modemType = parsedModemType;
        } else if (!json["modemType"].isNull()) {
            return false;
        }

        if (json["apn"].is<String>()) {
            apn = json["apn"].as<String>();
        } else if (!json["apn"].isNull()) {
            return false;
        }

        if (json["pin"].is<String>()) {
            pin = json["pin"].as<String>();
            pinConfigured = !pin.isEmpty();
        } else if (!json["pin"].isNull()) {
            return false;
        }

        if (json["pinConfigured"].is<bool>()) {
            pinConfigured = json["pinConfigured"].as<bool>();
        } else if (!json["pinConfigured"].isNull()) {
            return false;
        }

        if (json["smsEnabled"].is<bool>()) {
            smsEnabled = json["smsEnabled"].as<bool>();
        } else if (!json["smsEnabled"].isNull()) {
            return false;
        }

        return true;
    }
};
