#pragma once

#include "ConfigModelSupport.h"
#include "JsonConfig.h"

class PhoneWhitelistEntry : public JsonConfig {
public:
    String label;
    String phoneNumber;

    void toJSON(JsonObject json) const {
        json["label"] = label;
        json["phoneNumber"] = phoneNumber;
    }

    bool fromJSON(JsonObject json) {
        if (!json["label"].is<String>() || !json["phoneNumber"].is<String>()) {
            return false;
        }
        label = json["label"].as<String>();
        phoneNumber = json["phoneNumber"].as<String>();
        return true;
    }
};
